#pragma once

#include <iostream>

struct Pair {
	uint32_t from_id;
	uint32_t to_id;
	float distance;
};

// Swap two node's values.
static inline void
swap(Pair& node1, Pair& node2)
{
	Pair tmp = node1;
	node1 = node2;
	node2 = tmp;
}

static inline void
heapify_subroot(Pair *heap, const uint32_t& size, const uint32_t& subroot_index)
{
	uint32_t largest = subroot_index;
	uint32_t left = 2 * subroot_index + 1;
	uint32_t right = 2 * subroot_index + 2;

	if (left < size && heap[left].distance > heap[largest].distance)
		largest = left;
	if (right < size && heap[right].distance > heap[largest].distance)
		largest = right;

	if (largest != subroot_index) {
		swap(heap[subroot_index], heap[largest]);

		heapify_subroot(heap, size, largest);
	}
}

static inline void
heapify_insertee(Pair *heap, const uint32_t& size, const uint32_t& insertee_index)
{
	if (insertee_index == 0) return;

	uint32_t parent = (insertee_index - 1) / 2;

	if (heap[insertee_index].distance > heap[parent].distance) {
		swap(heap[insertee_index], heap[parent]);

		// Heapify the parent node.
		heapify_insertee(heap, size, parent);
	}
}

static inline Pair
heap_pop(Pair *heap, uint32_t& size)
{
	// The last item of the heap.
	Pair root = heap[0];

	// Replace root with last item.
	heap[0] = heap[--size];

	// Heapify the root node.
	heapify_subroot(heap, size, 0);

	return root;
}

static inline void
heap_insert(Pair *heap, uint32_t& size, const Pair& insertee)
{
	// Add the item, while increasing the size of the heap.
	heap[size++] = insertee;

	// Heapify the new item.
	heapify_insertee(heap, size, size - 1);
}

//static inline void
//heap_print(uint32_t *heap, const size_t& heap_size)
//{
//	printf("Printing heap with %zu items.\n", heap_size);
//
//	for (size_t i = 0; i < heap_size; ++i)
//		printf("%u ", heap[i]);
//	printf("\n");
//}
