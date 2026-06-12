// simple hash table implementation
template <unsigned int T, typename V>
    
    class hash {
    private:
        
        struct Bucket {
        public:
            Bucket() {
                m_next = nullptr;
                m_index = 0;
            }
            
            std::optional<V> m_value;
            Bucket* m_next;
            unsigned int m_index;
        };
        
    public:
        
        hash() {
            m_buckets.resize(T);
            fillBuckets();
        }
        
        ~hash() {
            clearBuckets();
        }
        
        
        void HashValue(V value) { m_hash(value); }
        void RemoveValue(unsigned int bucket, V value) { RemoveBucket(m_buckets[bucket], value); }
        unsigned long size() { return m_buckets.size(); }
        
        unsigned int index(V value) {
            if (this->amount(value) != 0) {
                return ComputeHash(value);
            }
            std::cout << "Value is not within the set." << std::endl;
            return 0;
        }
        
        Bucket* operator[](unsigned int index) {
            return m_buckets[index];
        }
        
        std::vector<V> getValues() {
            std::vector<V> vals;
            for (int i = 0; i < m_buckets.size(); i++) {
                Bucket* current = m_buckets[i];
                while (current != nullptr) {
                    if (current->m_value)
                        vals.push_back(*current->m_value);
                    current = current->m_next;
                }
            }
            return vals;
        }
        
        std::vector<V> getValues(unsigned int index) {
            std::vector<V> vals;
            Bucket* current = m_buckets[index];
            while (current != nullptr) {
                if (current->m_value)
                    vals.push_back(current->m_value.value());
                current = current->m_next;
            }
            return vals;
        }
        
        unsigned int amount(const V& value) {
            unsigned int count = 0;
            unsigned int index = ComputeHash(value);
            Bucket* current = m_buckets[index];
            while (current != nullptr) {
                if (current->m_value && current->m_value.value() == value)
                    count++;
                    
                if (current->m_next == nullptr)
                    break;
                current = current->m_next;
            }
            return count;
        }
        
        
        void printKeyValues() {
            for(int i = 0; i < m_buckets.size(); i++) {
                std::println("{} : {}", i, this->getValues(i));
            }
        }
        
        void printValues() {
            std::println("{}", this->getValues());
        }
        
    private:
        std::vector<Bucket*> m_buckets;
        int m_resizeFactor = 7;
        
        Bucket* m_hash(V value) {
            const unsigned int num = ComputeHash(value);
            
            if (!(m_buckets[num]->m_value)) {
                m_buckets[num]->m_value = value;
                m_buckets[num]->m_index = num;
                
                Attemptresize();
                
                return m_buckets[num];
            }
            
            Bucket* available_bucket = SearchBucket(m_buckets[num]);
            available_bucket->m_value = value;
            available_bucket->m_index = num;
            
            Attemptresize();
            
            return available_bucket;
        }
        
        void Attemptresize() {
            int count = 0;
            
            for (int i = 0; i < m_buckets.size(); i++) {
                if (m_buckets[i]->m_value) {
                    count += 1;
                }
            }
            
            if (count >= m_buckets.size() / 2) {
                unsigned long old_size = m_buckets.size();
                unsigned long new_size = old_size + (m_resizeFactor * 2);
                std::vector<V> old_values = getValues();
                clearBuckets();
                m_buckets.resize(new_size);
                fillBuckets();
                
                for (const auto& value : old_values)
                    m_hash(value);
                
                m_resizeFactor += 1;
            }
        }
        
        void clearBuckets() {
            for (int i = 0; i < m_buckets.size(); i++) {
                while (m_buckets[i]->m_next != nullptr) {
                    Bucket* del = m_buckets[i];
                    m_buckets[i] = m_buckets[i]->m_next;
                    delete del;
                }
                delete m_buckets[i];
            }
        }
        
        void fillBuckets() {
            for (int i = 0; i < m_buckets.size(); i++) {
                m_buckets[i] = new Bucket;
            }
        }
        
        unsigned int ComputeHash(V value) {
            std::hash<V> hasher;
            return static_cast<unsigned int>(hasher(value) % m_buckets.size());
        }
        
        Bucket* SearchBucket(Bucket* bucket) {
            while (bucket->m_next != nullptr) {
                bucket = bucket->m_next;
            }
            bucket->m_next = new Bucket;
            return bucket->m_next;
        }
        
        void RemoveBucket(Bucket* bucket, V value) {
            bool isFound = false;
            unsigned int pos = ComputeHash(value);
            
            
            if (bucket->m_value && bucket->m_value.value() == value) {
                if (bucket->m_next != nullptr) {
                    m_buckets[pos] = bucket->m_next;
                    delete bucket;
                }
                else if(bucket->m_next == nullptr && bucket->m_value) {
                    bucket->m_next = nullptr;
                    bucket->m_value.reset();
                }
                return;
            }
            
            
            while (bucket->m_next != nullptr && bucket->m_next->m_value) {
                if (bucket->m_next->m_value.value() == value) {
                    isFound = true;
                    break;
                }
                bucket = bucket->m_next;
            }
            
            if (isFound) {
                Bucket* dead_bucket = bucket->m_next;
                
                if (bucket->m_next->m_next) {
                    bucket->m_next = bucket->m_next->m_next;
                }
                else {
                    bucket->m_next = nullptr;
                }
                
                delete dead_bucket;
                return;
            }
            
            std::cout << "This value is not in the bucket" << std::endl;
            return;
            
        }
        
        
    };
