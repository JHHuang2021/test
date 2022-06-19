#include "train.h"

#include <unordered_map>

#include "lib/datetime.h"
#include "lib/exception.h"
#include "lib/hash.h"
#include "lib/tuple.h"
#include "lib/utils.h"

namespace lin {

namespace {
// constexpr const int INT_MAX = std::numeric_limits<int>::max();  // need
// #include<limits>
constexpr const int INT_MAX = 2147483647;
constexpr const int INT_MIN = -2147483648;
constexpr const auto kHashMin = 0UL;
constexpr const auto kHashMax = SIZE_MAX;
}  // namespace

TrainSeats::TrainSeats() {
    // throw Exception("Default Constructor of TrainSeats should not be used");
}
TrainSeats::TrainSeats(int initial_seat_num, int station_num) {
    std::fill(seat_num, seat_num + station_num, initial_seat_num);
}

TrainSeatsWrap::TrainSeatsWrap(int initial_seat_num, int station_num)
    : initial_seat_num(initial_seat_num), station_num(station_num) {}
TrainSeatsWrap::TrainSeatsWrap(TrainSeats seats)
    : TrainSeats(seats), exist(true) {}
TrainSeatsWrap::~TrainSeatsWrap() {
    // if (need_destruct) delete seats;
}
int TrainSeatsWrap::RangeMin(int l, int r) {
    if (exist) {
        return TrainSeats::RangeMin(l, r);
    } else {
        return initial_seat_num;
    }
}
void TrainSeatsWrap::RangeAdd(int l, int r, int x) {
    if (!exist) {
        exist = true;
        std::fill(seat_num, seat_num + station_num, initial_seat_num);
        // seats = new TrainSeats(initial_seat_num, station_num);
        // need_destruct = true;
    }
    TrainSeats::RangeAdd(l, r, x);
}
int TrainSeatsWrap::operator[](int i) {
    if (exist) return seat_num[i];
    return initial_seat_num;
}

int TrainSeats::RangeMin(int l, int r) {
    int res = INT_MAX;
    for (int i = l; i < r; ++i) res = std::min(res, seat_num[i]);
    return res;
}
void TrainSeats::RangeAdd(int l, int r, int x) {
    for (int i = l; i < r; ++i) seat_num[i] += x;
}

bool CompareTime(const Ticket &a, const Ticket &b) {
    if (a.duration == b.duration) return a.train_id < b.train_id;
    return a.duration < b.duration;
}
bool CompareCost(const Ticket &a, const Ticket &b) {
    if (a.cost == b.cost) return a.train_id < b.train_id;
    return a.cost < b.cost;
}

TransferTicket::TransferTicket() : duration(INT_MAX), cost(INT_MAX) {}
bool CompareTime(const TransferTicket &a, const TransferTicket &b) {
    if (a.duration == b.duration) {
        if (a.cost == b.cost) {
            if (a.ticket1.train_id == b.ticket2.train_id) {
                return a.ticket2.train_id < b.ticket2.train_id;
            } else {
                return a.ticket1.train_id < b.ticket1.train_id;
            }
        } else {
            return a.cost < b.cost;
        }
    } else {
        return a.duration < b.duration;
    }
    // return Tuple(a.duration, a.cost, a.ticket1.train_id, a.ticket2.train_id)
    // <
    //        Tuple(b.duration, b.cost, b.ticket1.train_id, b.ticket2.train_id);
}
bool CompareCost(const TransferTicket &a, const TransferTicket &b) {
    if (a.cost == b.cost) {
        if (a.duration == b.duration) {
            if (a.ticket1.train_id == b.ticket2.train_id) {
                return a.ticket2.train_id < b.ticket2.train_id;
            } else {
                return a.ticket1.train_id < b.ticket1.train_id;
            }
        } else {
            return a.duration < b.duration;
        }
    } else {
        return a.cost < b.cost;
    }
    // return Tuple(a.cost, a.duration, a.ticket1.train_id, a.ticket2.train_id)
    // <
    //        Tuple(b.cost, b.duration, b.ticket1.train_id, b.ticket2.train_id);
}

std::string Order::ToString() const {
    std::string ret;
    switch (status) {
        case SUCCESS:
            ret = "[success]";
            break;
        case PENDING:
            ret = "[pending]";
            break;
        case REFUNDED:
            ret = "[refunded]";
            break;
        default:
            throw Exception();
            break;
    }
    append(ret, ' ', train_id.c_str(), ' ', from_station.c_str(), ' ',
           dep_datetime.ToString(), " -> ", to_station.c_str(), ' ',
           arr_datetime.ToString(), ' ', std::to_string(cost), ' ',
           std::to_string(num));
    return ret;
}

std::string TrainManager::AddTrain(int timestamp, const Train &train) {
    auto train_id_hash = TrainIdHasher(train.id);
    bool exist = trains_.GetValue(train_id_hash).first;
    if (exist) return "-1";
    trains_.Insert(train_id_hash, train, timestamp);
    return "0";
}

std::string TrainManager::DeleteTrain(int timestamp,
                                      std::string_view train_id) {
    auto train_id_hash = TrainIdHasher(train_id);
    auto [exist, train] = trains_.GetValue(train_id_hash);
    if (!exist) return "-1";          // 车次不存在
    if (train.released) return "-1";  // 不能删除已发布的车次
    trains_.Remove(train_id_hash, timestamp);
    return "0";
}

std::string TrainManager::ReleaseTrain(int timestamp,
                                       std::string_view train_id) {
    auto train_id_hash = TrainIdHasher(train_id);
    auto [exist, train] = trains_.GetValue(train_id_hash);
    if (!exist) return "-1";          // 车次不存在
    if (train.released) return "-1";  // 不可重复 release
    train.released = true;
    for (int i = 0; i < train.station_num; ++i) {
        station_trains_.Insert(
            std::make_pair(StationHasher(train.stations[i]), train_id_hash),
            StationTrain(train_id_hash, train.arrival_times[i],
                         train.departure_times[i], train.sum_prices[i], i,
                         train),
            timestamp);
    }
    trains_.Modify(train_id_hash, train, timestamp);
    return "0";
}

std::string TrainManager::QueryTrain(int timestamp, std::string_view train_id,
                                     Date target_date) {
    auto train_id_hash = TrainIdHasher(train_id);
    auto [exist, train] = trains_.GetValue(train_id_hash);
    if (!exist) return "-1";  // 车次不存在
    if (target_date < train.start_sale || train.end_sale < target_date)
        return "-1";  // 超出日期范围
    std::string ret;
    append(ret, train_id, ' ', train.type, '\n');
    auto seats =
        GetSeats(train_id_hash, target_date, train.seat_num, train.station_num);
    append(ret, train.stations[0].c_str(), " xx-xx xx:xx -> ",               //
           DateTime(target_date, train.departure_times[0]).ToString(), ' ',  //
           std::to_string(train.sum_prices[0]), ' ', std::to_string(seats[0]),
           '\n');
    for (int i = 1; i < train.station_num - 1; ++i) {
        append(ret, train.stations[i].c_str(), ' ',  //
               DateTime(target_date, train.arrival_times[i]).ToString(), " -> ",
               DateTime(target_date, train.departure_times[i]).ToString(),
               ' ',  //
               std::to_string(train.sum_prices[i]), ' ',
               std::to_string(seats[i]), '\n');
    }
    append(ret, train.stations[train.station_num - 1].c_str(), ' ',
           DateTime(target_date, train.arrival_times[train.station_num - 1])
               .ToString(),
           " -> xx-xx xx:xx ",
           std::to_string(train.sum_prices[train.station_num - 1]), " x");
    return ret;
}

namespace {
// 仅供 std::map 临时使用，之后换成 B+ 树
template <class Key, class Value>
vector<Value> RangeFind(const std::map<Key, Value> &m, const Key &key1,
                        const Key &key2) {
    auto it = m.lower_bound(key1);
    vector<Value> ret;
    for (; it != m.end() && !(key2 < it->first); ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

// ComparisonOf<T> 表示类型 T
// 的比较函数指针类型，用于在同名函数中选择一种特定的重载。
template <class T>
using ComparisonOf = bool (*)(const T &, const T &);
}  // namespace

TrainSeatsWrap TrainManager::GetSeats(TrainIdHash train_id_hash, Date date,
                                      int initial_seat_num, int station_num) {
    auto [success, seats] =
        train_seats_.GetValue(std::make_pair(train_id_hash, date));
    if (success)
        return TrainSeatsWrap(seats);
    else
        return TrainSeatsWrap(initial_seat_num, station_num);
}
void TrainManager::UpdateSeats(int timestamp, TrainIdHash train_id_hash,
                               Date date, const TrainSeatsWrap &seats) {
    auto key = std::make_pair(train_id_hash, date);
    auto [success, old_seats] = train_seats_.GetValue(key);
    if (success)
        // *seats_ptr = *seats.seats;
        train_seats_.Modify(key, TrainSeats(seats), timestamp);
    else
        train_seats_.Insert(key, TrainSeats(seats), timestamp);
}

std::string TrainManager::QueryTicket(int timestamp, Date date,
                                      std::string_view from_station,
                                      std::string_view to_station,
                                      SortOrder sort_order) {
    auto from_hash = StationHasher(from_station),
         to_hash = StationHasher(to_station);
    vector<StationTrain> start_trains, end_trains;
    station_trains_.GetValue(std::make_pair(from_hash, kHashMin),
                             std::make_pair(from_hash, kHashMax),
                             &start_trains);
    if (start_trains.empty()) return "0";
    station_trains_.GetValue(std::make_pair(to_hash, kHashMin),
                             std::make_pair(to_hash, kHashMax), &end_trains);
    if (end_trains.empty()) return "0";
    vector<Ticket> result;
    for (auto i = start_trains.begin(), j = end_trains.begin();
         i != start_trains.end(); ++i) {
        while (j != end_trains.end() && j->train_id_hash < i->train_id_hash)
            ++j;
        if (j == end_trains.end()) break;
        if (i->train_id != j->train_id) continue;
        if (i->rank >= j->rank) continue;  // 列车运行方向不符
        Date start_date = date - i->departure_time.GetDays();
        if (start_date < i->start_sale || i->end_sale < start_date)
            continue;  // 超出售票日期
        auto seats =
            GetSeats(i->train_id_hash, start_date, i->seat_num, i->station_num);
        result.push_back(
            {i->train_id, start_date + i->departure_time,
             start_date + j->arrival_time, j->arrival_time - i->departure_time,
             j->sum_price - i->sum_price, seats.RangeMin(i->rank, j->rank)});
    }
    if (result.empty()) return "0";
    if (sort_order == SortOrder::TIME) {
        Sort(result.begin(), result.end(), ComparisonOf<Ticket>(CompareTime));
    } else {
        Sort(result.begin(), result.end(), ComparisonOf<Ticket>(CompareCost));
    }
    std::string ret = std::to_string(result.size());
    for (auto i : result) {
        append(ret, '\n', i.train_id.c_str(), ' ', from_station, ' ',
               i.start_time.ToString(), " -> ", to_station, ' ',
               i.end_time.ToString(), ' ', std::to_string(i.cost), ' ',
               std::to_string(i.seat));
    }
    return ret;
}

std::string TrainManager::QueryTransfer(int timestamp, Date date,
                                        std::string_view from_station,
                                        std::string_view to_station,
                                        SortOrder sort_order) {
    auto from_hash{StationHasher(from_station)},
        to_hash{StationHasher(to_station)};
    vector<StationTrain> start_trains, end_trains;
    station_trains_.GetValue(std::make_pair(from_hash, kHashMin),
                             std::make_pair(from_hash, kHashMax),
                             &start_trains);
    if (start_trains.empty()) return "0";
    station_trains_.GetValue(std::make_pair(to_hash, kHashMin),
                             std::make_pair(to_hash, kHashMax), &end_trains);
    if (end_trains.empty()) return "0";

    TransferTicket ans, cur;
    ans.duration = Duration(INT_MAX);
    ans.cost = INT_MAX;
    bool found = false;

    for (auto i : start_trains) {
        Date i_start_date = date - i.departure_time.GetDays();
        if (i_start_date < i.start_sale || i.end_sale < i_start_date) continue;
        Train i_train = trains_.GetValue(i.train_id_hash).second;
        std::unordered_map<StationHash, int> station_rank;
        for (int k = i.rank + 1; k < i_train.station_num; ++k)
            station_rank[StationHasher(i_train.stations[k])] = k;
        for (auto j : end_trains) {
            if (i.train_id == j.train_id) continue;
            Train j_train = trains_.GetValue(j.train_id_hash)
                                .second;  // TODO: 检查指针不会失效
            for (int k = 0; k < j.rank; ++k) {
                auto &transfer_station = j_train.stations[k];
                auto transfet_station_hash = StationHasher(transfer_station);
                auto iter = station_rank.find(transfet_station_hash);
                if (iter == station_rank.end()) continue;
                DateTime i_arr_datetime =
                    i_start_date + i_train.arrival_times[iter->second];
                auto [i_arr_date, i_arr_time] = i_arr_datetime.GetDateAndTime();
                auto [j_dep_days, j_dep_time] =
                    j_train.departure_times[k].GetDayTime();
                // j_start_date 为 j
                // 车可行的最早发车日期，由于可以在中转站停留任意长的时间，故只需要计算最早发车日期
                Date j_start_date = i_arr_date - j_dep_days;
                if (j_dep_time < i_arr_time) j_start_date += kOneDay;
                j_start_date = std::max(
                    j_start_date,
                    j_train.start_sale);  // 发车日期不能早于开始售票的日期
                if (j.end_sale < j_start_date) continue;

                DateTime j_dep_datetime =
                    j_start_date + j_train.departure_times[k];
                cur.ticket1.train_id = i.train_id;
                cur.ticket1.cost =
                    i_train.sum_prices[iter->second] - i.sum_price;
                cur.ticket2.train_id = j.train_id;
                cur.ticket2.cost = j.sum_price - j_train.sum_prices[k];
                cur.cost = cur.ticket1.cost + cur.ticket2.cost;
                cur.duration = (i_train.arrival_times[iter->second] -
                                i_train.departure_times[i.rank]) +
                               (j_train.arrival_times[j.rank] -
                                j_train.departure_times[k]) +
                               (j_dep_datetime - i_arr_datetime);

                bool is_better = sort_order == SortOrder::TIME
                                     ? CompareTime(cur, ans)
                                     : CompareCost(cur, ans);
                found = true;

                if (is_better) {
                    ans = cur;  // TODO: optimize
                    ans.transfer_station = transfer_station.str();
                    ans.ticket1.start_time = i_start_date + i.departure_time;
                    ans.ticket1.end_time = i_arr_datetime;
                    auto i_seats = GetSeats(i.train_id_hash, i_start_date,
                                            i.seat_num, i.station_num);
                    ans.ticket1.seat = i_seats.RangeMin(i.rank, iter->second);

                    ans.ticket2.start_time = j_dep_datetime;
                    ans.ticket2.end_time = j_start_date + j.arrival_time;
                    auto j_seats = GetSeats(j.train_id_hash, j_start_date,
                                            j.seat_num, j.station_num);
                    ans.ticket2.seat = j_seats.RangeMin(k, j.rank);
                }
            }
        }
    }
    if (!found) return "0";
    std::string ret;
    append(ret, ans.ticket1.train_id.c_str(), ' ', from_station, ' ',
           ans.ticket1.start_time.ToString(), " -> ", ans.transfer_station,
           ' ' + ans.ticket1.end_time.ToString(), ' ',
           std::to_string(ans.ticket1.cost), ' ',
           std::to_string(ans.ticket1.seat));
    append(ret, '\n', ans.ticket2.train_id.c_str(), ' ', ans.transfer_station,
           ' ', ans.ticket2.start_time.ToString(), " -> ", to_station,
           ' ' + ans.ticket2.end_time.ToString(), ' ',
           std::to_string(ans.ticket2.cost), ' ',
           std::to_string(ans.ticket2.seat));
    return ret;
}

std::string TrainManager::BuyTicket(int timestamp, std::string_view username,
                                    std::string_view train_id, Date date,
                                    const int number,
                                    std::string_view from_station,
                                    std::string_view to_station,
                                    const bool pending) {
    auto user_id_hash = UserIdHasher(username);
    auto train_id_hash = TrainIdHasher(train_id);
    auto from_hash = StationHasher(from_station),
         to_hash = StationHasher(to_station);
    auto [exist_from_st_train, from_st_train] =
        station_trains_.GetValue(std::make_pair(from_hash, train_id_hash));
    if (!exist_from_st_train) return "-1";
    Date start_date = date - from_st_train.departure_time.GetDays();
    if (start_date < from_st_train.start_sale ||
        from_st_train.end_sale < start_date || from_st_train.seat_num < number)
        return "-1";
    auto [exist_to_st_train, to_st_train] =
        station_trains_.GetValue(std::make_pair(to_hash, train_id_hash));
    if (!exist_to_st_train) return "-1";
    if (from_st_train.rank >= to_st_train.rank) return "-1";
    TrainSeatsWrap seats =
        GetSeats(train_id_hash, start_date, from_st_train.seat_num,
                 from_st_train.station_num);
    int avail_seats = seats.RangeMin(from_st_train.rank, to_st_train.rank);
    if (avail_seats < number && !pending) return "-1";
    /*
    struct Order {
      Status status;
      int timestamp, cost, num;
      Date start_date;
      User::IdType username;
      Train::IdType train_id;
      Train::StationName from_station, to_station;
      int from_rank, to_rank;
      DateTime dep_datetime, arr_datetime;
    };
    */
    Order order = {Order::Status::SUCCESS,
                   timestamp,  //
                   to_st_train.sum_price - from_st_train.sum_price,
                   number,  //
                   start_date,
                   username,
                   train_id,  //
                   from_station,
                   to_station,
                   from_st_train.rank,
                   to_st_train.rank,  //
                   start_date + from_st_train.departure_time,
                   start_date + to_st_train.arrival_time};
    std::string ret;
    if (avail_seats >= number) {
        seats.RangeAdd(from_st_train.rank, to_st_train.rank, -number);
        UpdateSeats(timestamp, train_id_hash, start_date, seats);
        ret = std::to_string(1ll * number *
                             (to_st_train.sum_price - from_st_train.sum_price));
    } else {
        order.status = Order::Status::PENDING;
        PendingOrder pending_order = {timestamp, number, from_st_train.rank,
                                      to_st_train.rank, user_id_hash};
        pending_orders_.Insert(Tuple(train_id_hash, start_date, timestamp),
                               pending_order, timestamp);
        ret = "queue";
    }
    orders_.Insert(std::make_pair(user_id_hash, -timestamp), order, timestamp);
    return ret;
}

std::string TrainManager::QueryOrder(int timestamp, std::string_view username) {
    auto user_id_hash = UserIdHasher(username);
    vector<Order> results;
    orders_.GetValue(std::make_pair(user_id_hash, INT_MIN),
                     std::make_pair(user_id_hash, 0), &results);
    std::string ret = std::to_string(results.size());
    for (auto order : results) append(ret, '\n', order.ToString());
    return ret;
}

std::string TrainManager::RefundTicket(int timestamp, std::string_view username,
                                       const int number) {
    auto user_id_hash = UserIdHasher(username);
    vector<Order> results;
    orders_.GetValue(std::make_pair(user_id_hash, INT_MIN),
                     std::make_pair(user_id_hash, 0), &results);
    if (number > results.size()) return "-1";
    Order &order = results[number - 1];
    if (order.status == Order::Status::REFUNDED) return "-1";

    Date start_date = order.start_date;
    auto train_id_hash = TrainIdHasher(order.train_id);

    if (order.status == Order::Status::PENDING) {
        pending_orders_.Remove(
            Tuple(train_id_hash, start_date, order.timestamp), timestamp);
    } else {
        auto seats =
            train_seats_.GetValue(std::make_pair(train_id_hash, start_date))
                .second;  // 买过票所以一定能查到
        seats.RangeAdd(order.from_rank, order.to_rank, order.num);
        vector<PendingOrder> pendings;
        pending_orders_.GetValue(Tuple(train_id_hash, start_date, 0),
                                 Tuple(train_id_hash, start_date, INT_MAX),
                                 &pendings);
        for (auto i : pendings)
            if (seats.RangeMin(i.from_rank, i.to_rank) >= i.num) {
                auto pending_order =
                    orders_
                        .GetValue(std::make_pair(i.user_id_hash, -i.timestamp))
                        .second;
                pending_order.status =
                    Order::Status::SUCCESS;  // TODO: 将修改写回文件。
                orders_.Modify(std::make_pair(i.user_id_hash, -i.timestamp),
                               pending_order, timestamp);
                seats.RangeAdd(i.from_rank, i.to_rank,
                               -i.num);  // TODO: 将修改写回文件。
                pending_orders_.Remove(
                    Tuple(train_id_hash, start_date, i.timestamp), timestamp);
            }
        train_seats_.Modify(std::make_pair(train_id_hash, start_date), seats,
                            timestamp);
    }
    order.status = Order::REFUNDED;
    // *orders_.GetValue(std::make_pair(user_id_hash, -order.timestamp)).first =
    // order;
    orders_.Modify(std::make_pair(user_id_hash, -order.timestamp), order,
                   timestamp);
    return "0";
}

void TrainManager::RollBack(int timestamp) {
    trains_.RollBack(timestamp);
    train_seats_.RollBack(timestamp);
    station_trains_.RollBack(timestamp);
    orders_.RollBack(timestamp);
    pending_orders_.RollBack(timestamp);
    return;
}

}  // namespace lin