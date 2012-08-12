#ifndef XLINKED_LIST_H
#define XLINKED_LIST_H

#include <stdint.h>
#include <stddef.h>

#include <sstream>
#include <stdexcept>

#define THROW_RTE(args) \
    do{ \
        std::stringstream ss; \
        ss << __FILE__ << ":" << __LINE__ << " - " << args; \
        throw std::runtime_error(ss.str()); \
    }while(0)

#define CHK_NULL(var) if((var) == NULL){ THROW_RTE("'" #var "' is NULL"); }


template<class T>
class xlinked_list {
    private:
        /* The integer type of a pointer, used in 
         * pointer arithmetic such as XOR
         */
        #if __WORDSIZE == 32
        typedef uint32_t ptr_int;
        #elif __WORDSIZE == 64
        typedef uint64_t ptr_int;
        #else
        #error Unsupported word size
        #endif

        /* A node in the linked list.  
         * The list is doubly-linked, with the forward and backward
         * links xor'd together.
         */
        typedef struct xll_node {
            ptr_int ptr;
            T *data;

            /* Node Constructors */
            //only used in creation of sentinel nodes
            xll_node() :ptr(0), data(NULL) { }

            //Avoid the need for T to support default construction
            //by using placement new and copy construction instead
            xll_node(ptr_int p, const T& d)
                :ptr(p), data(static_cast<T *>(::operator new(sizeof(T))))
            {
                data = new(data) T(d);
            }

            /* Node Destructor */
            ~xll_node() {
                if(data != NULL) delete data;
            }
        } xll_node;

    public:
        /***************************** *structors *****************************/

        /* Default constructor */
        xlinked_list()
            :head(NULL),
            tail(NULL),
            num_nodes(0)
        {
            head = new xll_node();
            tail = new xll_node();

            head->ptr = reinterpret_cast<ptr_int>(tail);
            tail->ptr = reinterpret_cast<ptr_int>(head);
        }

        /* Copy constructor */
        xlinked_list(const xlinked_list& other)
            :head(other.head),
            tail(other.tail),
            num_nodes(other.num_nodes)
        {
        }

        /* Default destructor */
        ~xlinked_list()
        {
            xll_node *cur = NULL;
            xll_node *prev = NULL;
            xll_node *del = NULL;
            xll_node *tmp = NULL;

            //Note: If either head or tail are NULL, then the list
            //has definitely been corrupted... memory leaks are better
            //than segfaults...
            if(head != NULL && tail != NULL){
                cur = reinterpret_cast<xll_node *>(head->ptr);
                prev = head;

                do {
                    del = prev;
                    tmp = reinterpret_cast<xll_node*>(reinterpret_cast<ptr_int>(prev) ^ cur->ptr);
                    prev = cur;
                    cur = tmp;
                    delete del;
                } while (cur != NULL);
            }
        }


        /***************************** Iterators *****************************/

        /* Iterator over a const xlinked_list */
        class const_iterator{
            public:
                const_iterator(xll_node *start, xll_node *prev_to_start)
                    :cur(start)
                    ,prev(prev_to_start)
                    ,forward(true) { }

                const_iterator(const const_iterator& other)
                    :cur(other.cur)
                    ,prev(other.prev)
                    ,forward(true) { }

                const_iterator operator++() { //pre-fix
                    if(!forward){
                        swap_direction();
                    }else{
                        iterate();
                    }

                    return (*this);
                }

                const_iterator operator++(int) { //post-fix
                    const_iterator copy(*this);
                    ++(*this);
                    return copy;
                }

                //Currently a NOP
                const_iterator operator--() { //pre-fix
                    if(forward){ 
                        swap_direction();
                    }else{
                        iterate();
                    }

                    return (*this);
                }

                const_iterator operator--(int) { //post-fix
                    const_iterator copy(*this);
                    --(*this);
                    return copy;
                }

                const T& operator*() const {
                    CHK_NULL(cur);
                    return *(cur->data);
                }

                T& operator*() {
                    CHK_NULL(cur);
                    return *(cur->data);
                }

                bool operator==(const const_iterator& other) const {
                    if(this == NULL){
                        if(&other == NULL) return true;
                        else return false;
                    }
                    if(&other == NULL) return false;
                    return (cur == other.cur && prev == other.prev) ? true : false;
                }

                bool operator!=(const const_iterator& other) const {
                    return !(*this == other);
                }

            private:
                void iterate() {
                    //Sanity checks
                    CHK_NULL(cur);
                    CHK_NULL(prev);

                    xll_node *next = reinterpret_cast<xll_node*>(reinterpret_cast<ptr_int>(prev) ^ cur->ptr);
                    prev = cur;
                    cur = next;
                }

                void swap_direction() {
                    xll_node *tmp = cur;
                    cur = prev;
                    prev = tmp;

                    forward = (forward) ? false : true;
                }

                xll_node *get_prev_node() { return prev; }

                mutable xll_node *cur;
                mutable xll_node *prev;
                bool forward;
        };

        /* Iterator over a xlinked_list */
        typedef const_iterator iterator;

        /* Get a forward-iterator starting at the head of the list.
         * This is the initial state of a forward-iterator.
         */
        iterator begin() { return iterator(reinterpret_cast<xll_node *>(head->ptr), head); }

        /* Get a forward-iterator over a const xlinked_list starting at the head of the list.
         * This is the initial state of a forward-iterator over a const xlinked_list.
         */
        const_iterator begin() const { return const_iterator(reinterpret_cast<xll_node *>(head->ptr), head); }

        /* Get a backward-iterator starting at the tail of the list.
         * This is the initial state of a backward-iterator.
         */
        iterator rbegin() { return iterator(reinterpret_cast<xll_node *>(tail->ptr), tail); }

        /* Get a backward-iterator over a const xlinked_list starting at the tail of the list.
         * This is the initial state of a backward-iterator over a const xlinked_list.
         */
        const_iterator rbegin() const { return const_iterator(reinterpret_cast<xll_node *>(tail->ptr), tail); }

        /* Get a forward-iterator starting at the tail of the list.
         * This is the 'fully-iterated' state of a forward-iterator.
         */
        iterator end() { return iterator(tail, reinterpret_cast<xll_node *>(tail->ptr)); }

        /* Get a forward-iterator over a const xlinked_list starting at the tail of the list.
         * This is the 'fully-iterated' state of a forward-iterator over a const xlinked_list.
         */
        const_iterator end() const { return const_iterator(tail, reinterpret_cast<xll_node *>(tail->ptr)); }

        /* Get a backward-iterator starting at the head of the list.
         * This is the 'fully-iterated' state of a backward-iterator.
         */
        iterator rend() { return iterator(head, reinterpret_cast<xll_node *>(head->ptr)); }

        /* Get a backward-iterator over a const xlinked_list starting at the head of the list.
         * This is the 'fully-iterated' state of a backward-iterator over a const xlinked_list.
         */
        const_iterator rend() const { return const_iterator(head, reinterpret_cast<xll_node *>(head->ptr)); }


        /***************************** Accessors *****************************/

        /* Return the first item in the list.
         * Throws a runtime_exception if list is empty.
         */
        const T& front() const {
            xll_node *node_1 = NULL;

            //Sanity checks
            CHK_NULL(head);
            if(head->ptr == 0){ THROW_RTE("xlinked_list has been corrupted: tail points to NULL"); }

            node_1 = reinterpret_cast<xll_node *>(head->ptr);
            if(node_1 == tail){ THROW_RTE("List is empty!"); }

            return *(node_1->data);
        }

        T& front() {
            return const_cast<T&>(const_cast<const xlinked_list<T>*>(this)->front());
        }

        /* Return the last item in the list.
         * Throws a runtime_exception if list is empty.
         */
        const T& back() const {
            xll_node *node_1 = NULL;

            //Sanity checks
            CHK_NULL(tail);
            if(tail->ptr == 0){ THROW_RTE("xlinked_list has been corrupted: tail points to NULL"); }

            node_1 = reinterpret_cast<xll_node *>(tail->ptr);
            if(node_1 == head){ THROW_RTE("List is empty!"); }

            return *(node_1->data);
        }

        T& back() {
            return const_cast<T&>(const_cast<const xlinked_list<T>*>(this)->back());
        }

        /* Get the length of the list */
        size_t size() const { return num_nodes; }


        /***************************** Modifiers *****************************/

        /* Note that use of the Modifiers will invalidate any existing iterators */

        /* Insert an item at the front of the list */
        void push_front(const T& item) {
            xll_node *node_1 = NULL;
            xll_node *node_2 = NULL;
            xll_node *node_new = NULL;

            //Sanity checks
            CHK_NULL(head);
            if(head->ptr == 0){ THROW_RTE("xlinked_list has been corrupted: head points to NULL"); }

            //Allocate node to be inserted
            node_new = new xll_node(0, item);

            //Init node vars
            node_1 = reinterpret_cast<xll_node *>(head->ptr);
            node_2 = reinterpret_cast<xll_node *>(node_1->ptr ^ reinterpret_cast<ptr_int>(head));

            //Patch up links
            node_1->ptr = reinterpret_cast<ptr_int>(node_new) ^ reinterpret_cast<ptr_int>(node_2);
            node_new->ptr = reinterpret_cast<ptr_int>(head) ^ reinterpret_cast<ptr_int>(node_1);
            head->ptr = static_cast<ptr_int>(0) ^ reinterpret_cast<ptr_int>(node_new);

            //Increment size
            num_nodes++;
        }

        /* Insert an item at the back of the list */
        void push_back(T& item) {
            xll_node *node_1 = NULL;
            xll_node *node_2 = NULL;
            xll_node *node_new = NULL;

            //Sanity checks
            CHK_NULL(tail);
            if(tail->ptr == 0){ THROW_RTE("xlinked_list has been corrupted: tail points to NULL"); }

            //Allocate node to be inserted
            node_new = new xll_node(0, item);

            //Init node vars
            node_1 = reinterpret_cast<xll_node *>(tail->ptr);
            node_2 = reinterpret_cast<xll_node *>(node_1->ptr ^ reinterpret_cast<ptr_int>(tail));

            //Patch up links
            node_1->ptr = reinterpret_cast<ptr_int>(node_new) ^ reinterpret_cast<ptr_int>(node_2);
            node_new->ptr = reinterpret_cast<ptr_int>(tail) ^ reinterpret_cast<ptr_int>(node_1);
            tail->ptr = static_cast<ptr_int>(0) ^ reinterpret_cast<ptr_int>(node_new);

            //Increment size
            num_nodes++;
        }

        /* Delete the first item in the list.
         * Throws a runtime_exception if list is empty.
         */
        void pop_front() {
            xll_node *node_1 = NULL;
            xll_node *node_2 = NULL;
            xll_node *node_3 = NULL;

            //Sanity checks
            CHK_NULL(head);
            if(head->ptr == 0){ THROW_RTE("xlinked_list has been corrupted: head points to NULL"); }

            node_1 = reinterpret_cast<xll_node *>(head->ptr);
            node_2 = reinterpret_cast<xll_node *>(node_1->ptr ^ reinterpret_cast<ptr_int>(head));
            if(node_2) node_3 = reinterpret_cast<xll_node *>(node_2->ptr ^ reinterpret_cast<ptr_int>(node_1));

            //If the list is already empty, then just return
            if(node_1 == tail){ THROW_RTE("List is empty!"); }

            //Patch up links
            head->ptr = reinterpret_cast<ptr_int>(node_2);
            if(node_2) node_2->ptr = reinterpret_cast<ptr_int>(head) ^ reinterpret_cast<ptr_int>(node_3);

            delete node_1;
            
            num_nodes--;
        }

        /* Delete the last item in the list */
        void pop_back() {
            xll_node *node_1 = NULL;
            xll_node *node_2 = NULL;
            xll_node *node_3 = NULL;

            //Sanity checks
            CHK_NULL(tail);
            if(tail->ptr == 0){ THROW_RTE("xlinked_list has been corrupted: tail points to NULL"); }

            node_1 = reinterpret_cast<xll_node *>(tail->ptr);
            node_2 = reinterpret_cast<xll_node *>(node_1->ptr ^ reinterpret_cast<ptr_int>(tail));
            if(node_2) node_3 = reinterpret_cast<xll_node *>(node_2->ptr ^ reinterpret_cast<ptr_int>(node_1));

            //If the list is already empty, then just return
            if(node_1 == head){ THROW_RTE("List is empty!"); }

            //Patch up links
            tail->ptr = reinterpret_cast<ptr_int>(node_2);
            if(node_2) node_2->ptr = reinterpret_cast<ptr_int>(tail) ^ reinterpret_cast<ptr_int>(node_3);

            delete node_1;

            num_nodes--;
        }

        /* Remove all content */
        void clear() {
            xll_node *cur = NULL;
            xll_node *prev = NULL;

            //Sanity checks
            CHK_NULL(head);
            CHK_NULL(tail);

            cur = reinterpret_cast<xll_node *>(head->ptr);
            prev = cur;
            cur = reinterpret_cast<xll_node *>(cur->ptr);

            while(cur != tail){
                delete cur;
                prev = cur;
                cur = reinterpret_cast<xll_node *>(cur->ptr);
            }

            //Reset list to empty
            num_nodes = 0;
            head->ptr = reinterpret_cast<ptr_int>(tail);
            tail->ptr = reinterpret_cast<ptr_int>(head);
        }


        /***************************** Operations *****************************/

        void reverse() {
            xll_node *tmp = head;
            head = tail;
            tail = tmp;
        }

    private:
        xll_node *head;
        xll_node *tail;
        size_t num_nodes;
};

#endif /* XLINKED_LIST_H */

