
class Base {
 public:
  Base(int minutes = 0) : minutes_(minutes) {}
  friend bool operator==(const Base &a, const Base &b) { return a.minutes_ == b.minutes_; }
  friend Base operator+(const Base &a, const Base &b) { return Base(a.minutes_ + b.minutes_); }

 private:
  int minutes_;
};

class Time : Base {
 public:
  Time(int minutes = 0) : Base(minutes){};
};

int main() {
  Time t1(2), t2;
  t1 == t2;
  return 0;
}
