
#include <stdio.h>
#include <math.h>
#include "NodeList.h"

/* ************************************************************************
   ListInitialize

   Purpose -      Initializes a List type.
   Parameters -   nextPrevOffset - offset from beginning of structure to
                                   the next and previous pointers with
                                   the structure that makes up the nodes.
                  orderFunction - used for sorting the list
   Returns -      None
   Side Effects -
   ----------------------------------------------------------------------- */
void ListInitialize(List* pList, int nextPrevOffset,
    int (*orderFunction)(void* pNode1, void* pNode2))
{
    pList->pHead = pList->pTail = NULL;
    pList->count = 0;
    pList->OrderFunction = orderFunction;
    pList->offset = nextPrevOffset;
}

/* ************************************************************************
   ListAddNode

   Purpose -      Adds a node to the end of the list.

   Parameters -   List *pList        -  pointer to the list
                  void *pStructToAdd -  pointer to the structure to add

   Returns -      none
   Side Effects -
   ----------------------------------------------------------------------- */
void ListAddNode(List* pList, void* pStructToAdd)
{
    ListNode* pTailNode;
    ListNode* pNodeToAdd;  // the next and prev pointers within proc to add
    int listOffset;

    listOffset = pList->offset;
    pNodeToAdd = (ListNode*)((unsigned char*)pStructToAdd + listOffset);
    pNodeToAdd->pNext = NULL;

    if (pList->pHead == NULL)
    {
        pList->pHead = pList->pTail = pStructToAdd;
        pNodeToAdd->pPrev = NULL;
    }
    else
    {
        // point to the list within proc_ptr
        pTailNode = (ListNode*)((unsigned char*)pList->pTail + listOffset);
        pTailNode->pNext = pStructToAdd;
        pNodeToAdd->pPrev = pList->pTail;
        pNodeToAdd->pNext = NULL;
        pList->pTail = pStructToAdd;
    }
    pList->count++;
}

/* ************************************************************************
   ListAddNodeInOrder

   Purpose -      Adds a node to the list based on the order function

   Parameters -   List *pList        -  pointer to the list
                  void *pStructToAdd -  pointer to the structure to add

   Returns -      none
   Side Effects -
   ----------------------------------------------------------------------- */
void ListAddNodeInOrder(List* pList, void* pStructToAdd)
{
    ListNode* pCurrentNode;
    ListNode* pNodeToAdd;  // the next and prev pointers within proc to add
    void* pCurrentStructure = NULL;
    int listOffset;
    int positionFound = 0;

    // must have an order function
    if (pList->OrderFunction == NULL)
    {
        return;
    }

    listOffset = pList->offset;
    pNodeToAdd = (ListNode*)((unsigned char*)pStructToAdd + listOffset);
    pNodeToAdd->pNext = NULL;

    if (pList->pHead == NULL)
    {
        pList->pHead = pList->pTail = pStructToAdd;
        pNodeToAdd->pPrev = NULL;
        pList->count++;
    }
    else
    {
        // start at the beginning
        pCurrentNode = (ListNode*)((unsigned char*)pList->pHead + listOffset);

        // traverse the list looking for the insertion place.
        while (pCurrentNode != NULL && !positionFound)
        {
            // keep a pointer to the structure         
            pCurrentStructure = (unsigned char*)pCurrentNode - listOffset;

            if ((pList->OrderFunction(pCurrentStructure, pStructToAdd) < 0) ||
                (pCurrentNode == NULL))
            {
                positionFound = 1;
            }
            else
            {
                /* if we are not at the end of the list, then move to the next node */
                if (pCurrentNode->pNext != NULL)
                {
                    pCurrentNode = (ListNode*)((unsigned char*)pCurrentNode->pNext + listOffset);
                }
                else
                {
                    pCurrentNode = NULL;
                }
            }
        }

        if (pCurrentNode == NULL)
        {
            // add to the end of the list
            ListAddNode(pList, pStructToAdd);
        }
        else
        {
            // insert right before the current node
            pNodeToAdd->pNext = pCurrentStructure;
            pNodeToAdd->pPrev = pCurrentNode->pPrev;
            pCurrentNode->pPrev = pStructToAdd;

            // move the head pointer if needed
            if (pNodeToAdd->pPrev == NULL)
            {
                pList->pHead = pStructToAdd;
            }
            else
            {
                ListNode* pPrevNode;
                pPrevNode = (ListNode*)((unsigned char*)pNodeToAdd->pPrev + listOffset);
                pPrevNode->pNext = pStructToAdd;
            }
            pList->count++;

        }
    }
}

/* ************************************************************************
   ListRemoveNode

   Purpose -      Removes the specified node from the list.

   Parameters -   List *pList           -  pointer to the list
                  void *pStructToRemove -  pointer to the structure to remove

   Returns -      None

   Side Effects -
   ----------------------------------------------------------------------- */
void ListRemoveNode(List* pList, void* pStructToRemove)
{
    ListNode* pPrevNode = NULL;
    ListNode* pNextNode = NULL;
    ListNode* pNodeToRemove;  // the next and prev pointers within proc to add
    int listOffset;

    listOffset = pList->offset;
    if (pList->count > 0)
    {
        pNodeToRemove = (ListNode*)((unsigned char*)pStructToRemove + listOffset);

        // if this is not the head and
        // prev and next are NULL, then the node is not on the list
        if (pList->pHead == pStructToRemove || pNodeToRemove->pNext != NULL || pNodeToRemove->pPrev != NULL)
        {
            if (pNodeToRemove->pPrev != NULL)
            {
                pPrevNode = (ListNode*)((unsigned char*)pNodeToRemove->pPrev + listOffset);
            }
            if (pNodeToRemove->pNext != NULL)
            {
                pNextNode = (ListNode*)((unsigned char*)pNodeToRemove->pNext + listOffset);
            }

            if (pPrevNode != NULL && pNextNode != NULL)
            {
                pPrevNode->pNext = pNodeToRemove->pNext;
                pNextNode->pPrev = pNodeToRemove->pPrev;
            }
            else
            {
                if (pPrevNode == NULL)
                {
                    /* replace the first node */
                    pList->pHead = pNodeToRemove->pNext;
                    if (pList->pHead)
                    {
                        pNextNode->pPrev = NULL;
                    }
                }
                if (pNextNode == NULL)
                {
                    /* replace the tail */
                    pList->pTail = pNodeToRemove->pPrev;
                    if (pList->pTail)
                    {
                        pPrevNode->pNext = NULL;
                    }
                }
            }
            pList->count--;

            pNodeToRemove->pNext = NULL;
            pNodeToRemove->pPrev = NULL;
        }
    }
}
/* ************************************************************************
   ListPopNode

   Purpose -      Removes the first node from the list and returns a pointer
                  to it.

   Parameters -   List *pList         -  pointer to the list

   Returns -      The function returns a pointer to the removed node.

   Side Effects -
   ----------------------------------------------------------------------- */
void* ListPopNode(List* pList)
{
    void* pNode = NULL;
    ListNode* pNodeToRemove;  // the next and prev pointers within proc to add
    int listOffset;

    listOffset = pList->offset;
    if (pList->count > 0)
    {
        pNodeToRemove = (ListNode*)((unsigned char*)pList->pHead + listOffset);

        pNode = pList->pHead;
        pList->pHead = pNodeToRemove->pNext;
        pList->pHead = pNodeToRemove->pNext;
        if (pNodeToRemove != NULL)
        {
            /* remove the previous pointer. */
            pNodeToRemove->pPrev = NULL;
        }

        pList->count--;

        // clear prev and next
        pNodeToRemove->pNext = NULL;
        pNodeToRemove->pPrev = NULL;
    }
    return pNode;
}


/* ************************************************************************
   ListGetNextNode

   Purpose -      Gets the next node releative to current node.  Returns
                  the first node if pCurrentStucture is NULL

   Parameters -   List *pList             -  pointer to the list
                  void *pCurrentStucture  -  pointer to current struct

   Returns -      The function returns a pointer to the next structure.

   Side Effects -
   ----------------------------------------------------------------------- */
void* ListGetNextNode(List* pList, void* pCurrentStucture)
{
    void* pNode = NULL;
    ListNode* pCurrentNode;  // Node part of the current structure
    int listOffset;

    if (pList != NULL)
    {
        if (pCurrentStucture == NULL)
        {
            pNode = pList->pHead;
        }
        else
        {
            listOffset = pList->offset;
            if (pList->count > 0)
            {
                pCurrentNode = (ListNode*)((unsigned char*)pCurrentStucture + listOffset);
                pNode = pCurrentNode->pNext;
            }
        }
    }
    return pNode;
}
/* ************************************************************************
   ListGetNextNode

   Purpose -      Gets the next node releative to current node.  Returns
                  the first node if pCurrentStucture is NULL

   Parameters -   List *pList             -  pointer to the list
                  void *pCurrentStucture  -  pointer to current struct

   Returns -      The function returns a pointer to the next structure.

   Side Effects -
   ----------------------------------------------------------------------- */
void* ListGetPreviousNode(List* pList, void* pCurrentStucture)
{
    void* pNode = NULL;
    ListNode* pCurrentNode;  // Node part of the current structure
    int listOffset;

    if (pList != NULL)
    {
        if (pCurrentStucture == NULL)
        {
            pNode = pList->pTail;
        }
        else
        {
            listOffset = pList->offset;
            if (pList->count > 0)
            {
                pCurrentNode = (ListNode*)((unsigned char*)pCurrentStucture + listOffset);
                pNode = pCurrentNode->pPrev;
            }
        }
    }
    return pNode;
}
/* ************************************************************************
   ListGetClosestNode

   Purpose -      Gets the closest node releative to current node.  Returns
                  the first node if pCurrentStucture is NULL

   Parameters -   List *pList             -  pointer to the list
                  void *pCurrentStucture  -  pointer to current struct

   Returns -      The function returns a pointer to the next structure.

   Side Effects -
   ----------------------------------------------------------------------- */
void* ListGetClosestNode(List* pList, void* pCurrentStucture)
{
    void* pNode = NULL;
    ListNode* pCurrentNode;  // Node part of the current structure
    int listOffset;

    // must have an order function
    if (pList->OrderFunction == NULL)
    {
        return NULL;
    }

    if (pList != NULL)
    {
        if (pCurrentStucture == NULL)
        {
            pNode = pList->pHead;
        }
        else
        {
            listOffset = pList->offset;
            if (pList->count > 0)
            {
                pCurrentNode = (ListNode*)((unsigned char*)pCurrentStucture + listOffset);
                if (pCurrentNode->pNext == NULL)
                {
                    pNode = pCurrentNode->pPrev;
                }
                else if (pCurrentNode->pPrev == NULL)
                {
                    pNode = pCurrentNode->pNext;
                }
                else
                {
                    int prevDistance;
                    int nextDistance;
                    prevDistance = abs(pList->OrderFunction(pCurrentStucture,
                        pCurrentNode->pPrev));
                    nextDistance = abs(pList->OrderFunction(pCurrentStucture,
                        pCurrentNode->pNext));
                    if (prevDistance < nextDistance)
                    {
                        pNode = pCurrentNode->pPrev;
                    }
                    else
                    {
                        pNode = pCurrentNode->pNext;
                    }
                }
            }
        }
    }
    return pNode;
}
