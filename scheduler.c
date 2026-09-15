#include "codexion.h"

// Compares two requests: FIFO -> earlier arrival wins, EDF ->
// earlier deadline wins. Returns 1 if a should be served before b.
int	has_priority(t_request *a, t_request *b, t_scheduler scheduler)
{
	if (scheduler == FIFO)
		return (a->arrival_time < b->arrival_time);
	return (a->deadline < b->deadline);
}

// Returns 1 if the two coders have a dongle in common
// (either one's left/right matches the other's left/right).
static int	shares_dongle(t_coder *a, t_coder *b)
{
	return (a->left == b->left || a->left == b->right
		|| a->right == b->left || a->right == b->right);
}

// Returns 1 if this coder is free to attempt an acquisition right
// now: no other pending request that wants one of the same
// dongles has higher priority. Fairness is scoped to shared
// dongles, not to the whole queue, so unrelated coders can still
// run concurrently.
int	is_front(t_queue *queue, t_coder *coder)
{
	t_request	*mine;
	int			i;

	i = 0;
	mine = NULL;
	while (i < queue->size && mine == NULL)
	{
		if (queue->requests[i].coder == coder)
			mine = &queue->requests[i];
		i++;
	}
	if (mine == NULL)
		return (0);
	i = 0;
	while (i < queue->size)
	{
		if (queue->requests[i].coder != coder
			&& shares_dongle(queue->requests[i].coder, coder)
			&& has_priority(&queue->requests[i], mine, coder->config->scheduler))
			return (0);
		i++;
	}
	return (1);
}

// Removes this coder's own request from wherever it sits in the
// heap array and restores the heap property.
void	queue_remove(t_queue *queue, t_coder *coder, t_scheduler scheduler)
{
	int	i;

	i = 0;
	while (i < queue->size && queue->requests[i].coder != coder)
		i++;
	if (i == queue->size)
		return ;
	queue->size--;
	queue->requests[i] = queue->requests[queue->size];
	sift_up(queue, i, scheduler);
	sift_down(queue, i, scheduler);
}
