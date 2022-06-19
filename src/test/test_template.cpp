
template <const size_t kSize>
class Char {
  char content_[kSize + 1] {'\0'};

 public:
  Char(const std::string &s) { strcpy(content_, s.c_str()); }
  Char(const char *cstr) { strcpy(content_, cstr); }
  char *c_str() { return content_; }
};


template <class T>
struct Hasher {
  size_t operator()(const T&) const;
};

template<>
template <int kSize>
struct Hasher<Char<kSize> > {
  size_t operator()(const Char<kSize>& str) const { return std::_Hash_impl::hash(str.c_str(), kSize); };
};