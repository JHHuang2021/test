#include "../train.h"
using namespace lin;
// bool CompareTime(const TransferTicket &a, const TransferTicket &b) {
//   return tie(a.duration, a.cost, a.ticket1.train_id, a.ticket2.train_id) <
//          tie(b.duration, b.cost, b.ticket1.train_id, b.ticket2.train_id);
// }
// bool CompareCost(const TransferTicket &a, const TransferTicket &b) {
//   return tie(a.cost, a.duration, a.ticket1.train_id, a.ticket2.train_id) <
//          tie(b.cost, b.duration, b.ticket1.train_id, b.ticket2.train_id);
// }

int main() {
  TransferTicket cur, ans;
  cur.duration = Duration(29333);
  cur.cost = 1385;
  std::cout << ans.duration.minutes() << " " << ans.cost << std::endl;
  auto t = tie(cur.duration, cur.cost);
  auto u = tie(ans.duration, ans.cost);
  std::cout << t.get<0>() << std::endl;
  std::cout << CompareTime(cur, ans) << std::endl;
  return 0;
}