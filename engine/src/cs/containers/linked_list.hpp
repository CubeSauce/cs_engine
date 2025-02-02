// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

template <typename Type>
class Linked_List
{
public:
    typedef struct Node
    {
        Type data;
        Node *next;

        Node(const Type& other)
            :   data(other), 
                next(nullptr) 
        {
        }
        
        Node(const Type& other, Node* other_next)
            :   data(other), 
                next(other_next) 
        {
        }
    } Node;

public:
    Linked_List()
        :   last(nullptr),
            first(nullptr),
            _size(0)
    {
    }

    Type& add(const Type& other)
    {
        if (last == nullptr)
        {
            first = last = new Node(other, last);
        }
        else
        {
            last = new Node(other, last);
        }

        _size += 1;

        return last->data;
    }

    Type* find(const Type& other)
    {
        Node* temp = last;
        while (temp != nullptr)
        {
            if (temp->data == other)
            {
                return &temp->data;
            }

            temp = temp->next;
        }

        return nullptr;
    }

    Type& find_or_add(const Type& other)
    {
        Node* temp = last;
        while (temp != nullptr)
        {
            if (temp->data == other)
            {
                return temp->data;
            }

            temp = temp->next;
        }

        return add(other);
    }

    bool contains(const Type& other)
    {
        Node* temp = last;
        while (temp != nullptr)
        {
            if (temp->data == other)
            {
                return true;
            }

            temp = temp->next;
        }

        return false;
    }

    void remove(const Type& other)
    {
        Node* temp = last;
        if (temp == nullptr)
        {
            return;
        }

        if(temp->data == other)
        {
            _size -= 1;
            last = temp->next;
            delete temp;
            return;
        }

        while(temp->next != nullptr)
        {
            if(temp->next->data == other)
            {
                _size -= 1;
                Node* del_node = temp->next;
                temp->next = temp->next->next;
                delete del_node;
                return;
            }

            temp = temp->next;
        }
    }

    void clear()
    {
        Node* temp;
        while (last != nullptr)
        {
            temp = last->next;
            delete last;
            last = temp;
        }
    }
    
    int32 get_size()
    {
        return _size;
    }

    Node* get_first()
    {
        return first;
    }

    Node* get_last()
    {
        return last;
    }

private:
    Node *last, *first;

    int32 _size;
};

template<typename Type>
using Linked_List_Node = Linked_List<Type>::Node;
