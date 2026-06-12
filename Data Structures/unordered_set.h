 template<typename V>
    
    class unordered_set {
    public:
        
        unordered_set() = default;
        
        unordered_set(std::initializer_list<V> values) {
            for (const auto& value : values) 
                m_hash.HashValue(value);
        }
        
        void insert(const V& value) {
            m_hash.HashValue(value);
        }
        
        void insert(std::initializer_list<V> values) {
            for (const auto& value : values)
                m_hash.HashValue(value);
        }
        
        
        void erase(const V& value) {
            m_hash.RemoveValue(m_hash.index(value), value);
        }
        
        void clear() {
            for (const auto& value : m_hash.getValues()) {
                m_hash.RemoveValue(m_hash.index(value), value);
            }
        }
    
        unsigned int count(const V& value) {
            return m_hash.amount(value);
        }
        
        unsigned int find(const V& value) {
            return m_hash.index(value);
        }
        
        std::vector<V> find(const unsigned int index) {
            return m_hash.getValues(index);
        }
        
        bool contains(const V& value) {
            return (this->count(value) > 0) ? true : false;
        }
        
        void print() {
            m_hash.printValues();
        }
        
        unsigned long size() {
            return m_hash.getValues().size();
        }
        
    
    private:
        // taken from my hash implementation
        self::hash<100, V> m_hash;
    };
