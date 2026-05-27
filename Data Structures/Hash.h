// Simple hash table implementation done in C++

struct Bucket {
public:
    Bucket() {
        m_value = nullptr;
        m_next = nullptr;
    }
    
    const char* m_value;
    Bucket* m_next;
};

template <unsigned int T>

class Hash {
public:
    
    Hash() {
        for (int i = 0; i < T; i++) {
            values[i] = new Bucket;
        }
    }
    
    ~Hash() {
        for (int i = 0; i < T; i++) {
            while (values[i] != nullptr) {
                Bucket* del = values[i];
                values[i] = values[i]->m_next;
                delete del;
            }
        }
    }
    
    
    unsigned int HashValue(const char* value) { return m_hash(value); }
    
    void RemoveValue(unsigned int bucket, const char* value) {
        
        RemoveBucket(values[bucket], value);
    }
    
    Bucket* find(unsigned int index) {
        return values[index];
    }
    
    bool lookup(const char* value) {
        unsigned int index = ComputeHash(value);
        Bucket* current = values[index];
        while (current != nullptr) {
            if (current->m_value == nullptr && current->m_next == nullptr)
                return false;
            
            if (current->m_value != nullptr && std::strcmp(current->m_value, value) == 0) {
                return true;
            }
            current = current->m_next;
        }
        return false;
    }
    
    unsigned int size() {
        return T;
    }
    
    unsigned int index(const char* value) {
        if (value != nullptr) {
            return ComputeHash(value);
        }
        return 0;
    }
    
   Bucket* operator[](unsigned int index) {
        return values[index];
    }
    
    std::vector<std::string> getValues() {
        std::vector<std::string> vals;
        for (int i = 0; i < this->size(); i++) {
            Bucket* current = values[i];
            while (current != nullptr) {
             if (current->m_value != nullptr)
                vals.push_back(current->m_value);
             current = current->m_next;
            }
        }
        return vals;
    }
    
    std::vector<std::string> getValues(unsigned int index) {
        std::vector<std::string> vals;
        Bucket* current = values[index];
        while (current != nullptr) {
            if (current->m_value != nullptr)
                vals.push_back(current->m_value);
            current = current->m_next;
        }
        return vals;
    }
    
    void insert(const char* value, unsigned int index) {
        Bucket* bucket = values[index];
        while (bucket->m_next != nullptr)
            bucket = bucket->m_next;
        
        Bucket* newBucket = new Bucket();
        newBucket->m_value = value;
        bucket->m_next = newBucket;
    }
    
  
    unsigned int insert(const char* value) {
        int emptyBucket = -1;
        Bucket* bucket = values[0];
        
        for (int i = 0; i < this->size(); i++) {
            if (values[i]->m_next == nullptr) {
                emptyBucket = i;
                bucket = values[i];
                break;
            }
        }
        
        if (emptyBucket == -1) {
            int smallestBucket = 0;
            int smallestLength = 1;
            for (int i = 0; i < this->size(); i++) {
                Bucket* current = values[i];
                Bucket* b = current;
                int temp_length = 0;
                
                while (current != nullptr) {
                    temp_length++;
                    if (current->m_next == nullptr)
                        b = current;
                    current = current->m_next;
                }
                
                if (temp_length < smallestLength) {
                    smallestLength = temp_length;
                    smallestBucket = i;
                    bucket = b;
                }
            }
            emptyBucket = smallestBucket;
        }
        
        Bucket* newbucket = new Bucket();
        newbucket->m_value = value;
        bucket->m_next = newbucket;
        
        return emptyBucket;
    }
    
    void printKeyValues() {
        for(int i = 0; i < this->size(); i++) {
            std::println("{} : {}", i, this->getValues(i));
        }
    }
    
private:
    Bucket* values[T];
    
    unsigned int m_hash(const char* value) {
        const unsigned int num = ComputeHash(value);
        
        if (values[num]->m_value == nullptr) {
            values[num]->m_value = value;
            return num;
        }
        
        Bucket* available_bucket = SearchBucket(values[num]);
        available_bucket->m_value = value;
        
        return num;
    }
    
    unsigned int ComputeHash(const char* value) {
        unsigned int num = 0;
        std::string val = static_cast<std::string>(value);
        
        for (int i = 0; i < val.size(); i++) {
            num += (std::tolower(val[i]) * 31);
        }
        
        return num % T;
    }
    
    Bucket* SearchBucket(Bucket* bucket) {
        while (bucket->m_next != nullptr) {
            bucket = bucket->m_next;
        }
        bucket->m_next = new Bucket;
        return bucket->m_next;
    }
    
    void RemoveBucket(Bucket* bucket, const char* value) {
        bool isFound = false;
        unsigned int pos = ComputeHash(value);
       
        
        if (std::strcmp(bucket->m_value, value) == 0) {
            if (bucket->m_next != nullptr) {
                values[pos] = bucket->m_next;
                delete bucket;
            }
            else if(bucket->m_next == nullptr && bucket->m_value != nullptr) {
                bucket->m_next = nullptr;
                bucket->m_value = nullptr;
            }
            return;
        }
        

        while (bucket->m_next != nullptr && bucket->m_next->m_value != nullptr) {
            if (std::strcmp(bucket->m_next->m_value, value) == 0) {
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
