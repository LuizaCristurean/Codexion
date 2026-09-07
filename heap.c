#include "codexion.h"

static void	swap_requests(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	sift_up(t_queue *queue, int i, t_scheduler scheduler)
{
	int parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (has_priority(&queue->requests[i], &queue->requests[parent], scheduler) == 0)
			break;
		swap_requests(&queue->requests[i], &queue->requests[parent]);
		i = parent;
	}
}

void	queue_push(t_queue *queue, t_request request, t_scheduler scheduler)
{
	queue->requests[queue->size] = request;
	sift_up(queue, queue->size, scheduler);
	queue->size++;
}

void	sift_down(t_queue *queue, int i, t_scheduler scheduler)
{
	int	left;
	int	right;
	int	best;

	while(1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		best = i;
		if (left < queue->size && has_priority(&queue->requests[left], &queue->requests[best], scheduler))
			best = left;
		if (right < queue->size && has_priority(&queue->requests[right], &queue->requests[best], scheduler))
    		best = right;
		if (best == i)
			break;
		swap_requests(&queue->requests[i], &queue->requests[best]);
		i = best;
    }
}

t_request	queue_pop(t_queue *queue, t_scheduler scheduler)
{
	t_request	top;

	top = queue->requests[0];
	queue->size--;
	queue->requests[0] = queue->requests[queue->size];
	sift_down(queue, 0, scheduler);
	return (top);
}

