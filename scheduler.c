#include "codexion.h"

int	has_priority(t_request *a, t_request *b, t_scheduler scheduler)
{
	if (scheduler == FIFO)
		return (a->arrival_time < b->arrival_time);
	return (a->deadline < b->deadline);
}

int	is_front(t_queue *queue, t_coder *coder)
{
	return (queue->size > 0 && queue->requests[0].coder == coder);
}

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