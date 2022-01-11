/* 	DIMITRIOS GREASIDIS 		1624	 */
/*	STEFANOS  PAPANASTASIOU 	1608	 */

/*
 * elevator clook
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct clook_data {
	struct list_head queue;
};

static void print_list(struct request_queue *q) {
	struct clook_data *cd = q->elevator->elevator_data;
	struct list_head *curr = NULL;
	struct request *rq = NULL;
	
	printk("[CLOOK] list: ");
	
	list_for_each(curr,&cd->queue) {
			rq = list_entry(curr, struct request, queuelist);
			printk(" %ld", (unsigned long)blk_rq_pos(rq));
	}
	printk("\n");
}

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *cd = q->elevator->elevator_data;
	struct list_head *curr = NULL;
	static sector_t last_sector = 0;
	int found;

	if (!list_empty(&cd->queue)) {
		struct request *rq;
		
		//rq = list_entry(nd->queue.next, struct request, queuelist);
		found = 0;
		list_for_each(curr, &cd->queue){
			rq = list_entry(curr, struct request, queuelist);
			if(blk_rq_pos(rq) >= last_sector) {
				found = 1;
				break;				
			}
		}
		if(!found){
			rq = list_entry(cd->queue.next, struct request, queuelist);
		}
		last_sector = blk_rq_pos(rq);
		
		if(rq_data_dir(rq) == 0) printk("[CLOOK] dsp R %ld\n", blk_rq_pos(rq));
		else printk("[CLOOK] dsp W %ld\n", blk_rq_pos(rq));
				
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		print_list(q);
		return 1;
	}
	return 0;
}

// rq ine to neo aitima
//q ine i i oura me ta aitimata pou prokeitai na eksipiretithoun
//prepei na mpainei mesa taksinomimeno me vasi ton arithmo ton sector
static void clook_add_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *cd = q->elevator->elevator_data;
	struct list_head *curr = NULL;
	struct request *pos_rq = NULL;
	
	//list_add_tail(&rq->queuelist, &nd->queue);
	if(list_empty(&cd->queue)) {
			list_add(&rq->queuelist, &cd->queue);
	}
	else {
			// find the correct position of the request in the list
			list_for_each(curr, &cd->queue){
				pos_rq = list_entry(curr, struct request, queuelist);
				if(blk_rq_pos(pos_rq) > blk_rq_pos(rq)) break; // correct posotion is when ti is greater than current request
			}
			list_add_tail(&rq->queuelist, curr);
	}
	
	if(rq_data_dir(rq) == 0) printk("[CLOOK] add R %ld\n", blk_rq_pos(rq));
	else printk("[CLOOK] add W %ld\n", blk_rq_pos(rq));
	print_list(q);
}

static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
clook_latter_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static void *clook_init_queue(struct request_queue *q)
{
	struct clook_data *nd;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd)
		return NULL;
	INIT_LIST_HEAD(&nd->queue);
	return nd;
}

static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_latter_req_fn		= clook_latter_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	elv_register(&elevator_clook);

	return 0;
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("DIM_STEF GRE_PAP");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CLOOK IO scheduler");
