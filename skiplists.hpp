#ifndef _SKIP_LISTS_HPP
#define _SKIP_LISTS_HPP

#include <vector>
#include <assert.h>

#include <time.h>

#define BITSINRANDOM 31


template<typename KeyType, typename ValType>
struct SkipListsNode {
    KeyType key;
    ValType value;

    std::vector<SkipListsNode *> forward;
};


template<typename KeyType, typename ValType>
class SkipLists {
    private:
        // maximum level of this list 
        // level = 0 of the list is empty
        int level;

        // the upper bound
        int max_number_of_levels;
        
        // max_number_of_levels - 1
        int max_level;

        int randoms_left;
        int random_bits;

        // pointer to header
        SkipListsNode<KeyType, ValType> * header;

    public:
        // ctor
        SkipLists(int max_level_num = 16) : 
            level(0), max_number_of_levels(max_level_num), max_level(max_number_of_levels - 1), randoms_left(BITSINRANDOM/2) {
                header = new SkipListsNode<KeyType, ValType>();
                assert(header != NULL);

                header->forward.resize(max_number_of_levels, NULL);
                // init the seed
                srand(time(NULL));
                random_bits = rand() % BITSINRANDOM + 1;
        }

        // destructor
        ~SkipLists() {
            SkipListsNode<KeyType, ValType>* p = header->forward[0];
            while(p != NULL) {
                SkipListsNode<KeyType, ValType>* next = p->forward[0];
                delete p;
                p = next;
            }

            header->forward.clear();
            
            delete header;
        }
        
        // generate radom level
        int random_level() {
            int l = 1;
            int b;
            do {
                b = random_bits & 3;
                if (!b) l++;
                random_bits >>= 2;
                if (--randoms_left == 0) {
                    random_bits = rand() % BITSINRANDOM + 1;
                    randoms_left = BITSINRANDOM / 2;
                }

            } while(!b);
            
            return (l > max_level ? max_level : l);

        }

        void print() {
            SkipListsNode<KeyType, ValType> * p;

            for(int i=level-1; i>=0; i--) { // for each level
                p = header->forward[i];

                while (p != NULL) {
                    std::cout << p->key << ":" << p->value << " ";
                    p = p->forward[i];
                }

                std::cout << std::endl;
            }

        }        

        bool insert(const KeyType& key, const ValType& value) {
            int k = level;

            SkipListsNode<KeyType, ValType>* update[max_number_of_levels];
            SkipListsNode<KeyType, ValType>* p = header;
            SkipListsNode<KeyType, ValType>* q;

            while ( --k >= 0) {
                while (q = p->forward[k], q != NULL && q->key < key) {
                    p = q;
                }
                update[k] = p;
            }

            if (q != NULL && q->key == key) {
                q->value = value;
                // insert the same value
                return false;
            }

            k = random_level();
            if (k > level) {
                k = ++level;
                // update index from 0
                update[k-1] = header;
            }
            q = new SkipListsNode<KeyType, ValType>();
            if (!q) {
                // out of memory
                return false;
            }
            q->forward.resize(k, NULL);
            q->key = key;
            q->value = value;

            while ( --k >= 0 ) {
                p = update[k];
                q->forward[k] = p->forward[k];
                p->forward[k] = q;
            }


            return true;
        }

        bool remove(const KeyType& key) {
            SkipListsNode<KeyType, ValType>* update[max_number_of_levels];
            SkipListsNode<KeyType, ValType>* p = header;
            SkipListsNode<KeyType, ValType>* q;

            int k = level;

            // search first
            while (--k >= 0) {
                while (q = p->forward[k], q->key < key) {
                    p = q;
                }

                update[k] = p;
            }

            if ((q != NULL) && (q->key == key)) {
                for(int i=0; (i<level) && (update[i]->forward[i] == q); ++i) {
                    p = update[i];
                    assert(p->forward[i]->key == key);
                    p->forward[i] = q->forward[i];
                }

                delete q;

                int m = level;
                while ( header->forward[m-1] == NULL && m >= 0) {
                    --m;
                }

                level = m;

                return true;
            }

            return false;
        }

        bool find(const KeyType& key, ValType& res) {
            SkipListsNode<KeyType, ValType>* p = header;
            SkipListsNode<KeyType, ValType>* q;

            int k = level;
            while (--k >= 0) {
                while (q = p->forward[k], q->key < key) {
                    p = q;
                }
            }

            if (q != NULL && q->key == key) {
                res = q->value;
                return true;
            }

            return false;
        }
};

#endif
