#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
// #include <vector>
#include "../lib/vector.h"
#include "bufferpool.hpp"
namespace huang {
template <class Key, class Value, int internal_size = 300, int leaf_size = 100>
class BPlusTree {
 public:
  BPlusTree(const std::string& name) {
    tree_filename = name + "tree.dat";
    leaf_filename = name + "leaf.dat";
    rollback_filename = name + "rollback.dat";

    tree_file.open(tree_filename);
    leaf_file.open(leaf_filename);
    rollback.open(rollback_filename);
    if (!leaf_file || !tree_file) {
      tree_file.open(tree_filename, std::ios::out);
      leaf_file.open(leaf_filename, std::ios::out);
      rollback.open(rollback_filename, std::ios::out);
      root.Set(1, 1, true);
      root.son[0] = 1;
      size = 0;
      rec = 0;
      last_internal = last_leaf = 1;

      Leaf leaff(0, 1, 0);
      WriteLeaf(leaff);
      tree_file.close();
      leaf_file.close();
      rollback.close();
      tree_file.open(tree_filename);
      leaf_file.open(leaf_filename);
      rollback.open(rollback_filename);
    } else {
      tree_file.seekg(0);
      int tree_rt;
      tree_file.read(reinterpret_cast<char*>(&tree_rt), sizeof(int));
      tree_file.read(reinterpret_cast<char*>(&last_internal), sizeof(int));
      tree_file.seekg(INTSIZE + tree_rt * sizeof(Internal));
      tree_file.read(reinterpret_cast<char*>(&root), sizeof(Internal));

      leaf_file.seekg(0);
      leaf_file.read(reinterpret_cast<char*>(&last_leaf), sizeof(int));
      leaf_file.read(reinterpret_cast<char*>(&size), sizeof(int));

      rollback.seekg(0);
      rollback.read(reinterpret_cast<char*>(&rec), sizeof(int));
    }
  }
  ~BPlusTree() {
    tree_file.seekp(0);
    tree_file.write(reinterpret_cast<char*>(&root.pos), sizeof(int));
    tree_file.write(reinterpret_cast<char*>(&last_internal), sizeof(int));
    WriteInternal(root);

    leaf_file.seekp(0);
    leaf_file.write(reinterpret_cast<char*>(&last_leaf), sizeof(int));
    leaf_file.write(reinterpret_cast<char*>(&size), sizeof(int));

    while (!internal_pool.Empty()) {
      Internal tmp = internal_pool.Pop();
      tree_file.seekp(tmp.pos * sizeof(Internal) + INTSIZE);
      tree_file.write(reinterpret_cast<char*>(&tmp), sizeof(Internal));
    }
    while (!leaf_pool.Empty()) {
      Leaf tmp = leaf_pool.Pop();
      leaf_file.seekp(tmp.pos * sizeof(Leaf) + INTSIZE);
      leaf_file.write(reinterpret_cast<char*>(&tmp), sizeof(Leaf));
    }

    //�ռ���� TODO
    rollback.seekp(0);
    rollback.write(reinterpret_cast<char*>(&rec), sizeof(int));

    tree_file.close();
    leaf_file.close();
    rollback.close();
  }
  bool Empty() { return size == 0; }
  void Insert(const Key& key, const Value& value, int timestamp,
              bool flag = false) {
    if (!flag) Record(Rollback::type::INSERT, timestamp, key, Value());
    if (InsertIfFatherSplit({key, value}, root)) {
      int m = internal_size / 2;
      Internal new_brother(m, GetInternalIndex(), root.is_leaf);
      Internal new_root(2, GetInternalIndex(), false);

      for (int i = 0; i < m; i++) new_brother.son[i] = root.son[m + i];
      for (int i = 0; i < m - 1; i++) new_brother.key[i] = root.key[m + i];
      root.num = m;
      WriteInternal(new_brother);
      WriteInternal(root);

      new_root.son[0] = root.pos;
      new_root.son[1] = new_brother.pos;
      new_root.key[0] = root.key[m - 1];
      root = new_root;

      WriteInternal(root);
    }
  }
  void Remove(const Key& key, int timestamp, bool flag = false) {
    if (!flag)
      Record(Rollback::type::REMOVE, timestamp, key, GetValue(key).second);
    if (RemoveIfFatherMerge(key, root)) {
      if (!root.is_leaf && root.num == 1) {
        Internal son;
        ReadInternal(son, root.son[0]);
        internal_pool.Remove(root.pos);
        root = son;
      }
    }
  }
  std::pair<bool, Value> GetValue(const Key& key) {
    Value ret;
    bool flag = true;
    Internal tmp = root;
    while (!tmp.is_leaf) {
      int pos = BinSearchInternalKey(key, tmp);
      ReadInternal(tmp, tmp.son[pos]);
    }
    int pos_leaf = BinSearchInternalKey(key, tmp);
    ReadLeaf(leaf, tmp.son[pos_leaf]);
    int pos = BinSearchLeafKey(key, leaf);
    if (pos == leaf.num || !(leaf.val[pos].first == key))
      flag = false;
    else
      ret = leaf.val[pos].second;
    return {flag, ret};
  }
  void GetValue(const Key& min_key, const Key& max_key,
                lin::vector<Value>* ans) {
    Internal tmp = root;
    while (!tmp.is_leaf) {
      int pos = BinSearchInternalKey(min_key, tmp);
      ReadInternal(tmp, tmp.son[pos]);
    }
    int pos_leaf = BinSearchInternalKey(min_key, tmp);
    ReadLeaf(leaf, tmp.son[pos_leaf]);
    while (true) {
      int i;
      for (i = 0; i < leaf.num; i++)
        if (min_key <= leaf.val[i].first) break;
      for (int j = i; j < leaf.num; j++)
        if (leaf.val[j].first <= max_key)
          ans->push_back(leaf.val[j].second);
        else
          return;
      if (!leaf.nxt)
        return;
      else
        ReadLeaf(leaf, leaf.nxt);
    }
  }
  void Modify(const Key& key, const Value& new_value, int timestamp,
              bool flag = false) {
    if (!flag)
      Record(Rollback::type::MODIFY, timestamp, key, GetValue(key).second);
    Internal tmp = root;
    while (!tmp.is_leaf) {
      int pos = BinSearchInternalKey(key, tmp);
      ReadInternal(tmp, tmp.son[pos]);
    }
    int pos_leaf = BinSearchInternalKey(key, tmp);
    ReadLeaf(leaf, tmp.son[pos_leaf]);
    int pos = BinSearchLeafKey(key, leaf);
    leaf.val[pos].second = new_value;
    WriteLeaf(leaf);
  }
  void RollBack(int timestamp) {
    for (int i = rec; i > 0; i--) {
      rollback.seekg(sizeof(int) + (i - 1) * sizeof(Rollback));
      rollback.read(reinterpret_cast<char*>(&roll), sizeof(Rollback));
      if (roll.timestamp <= timestamp) {
        rec = i;
        return;
      }
      if (roll.op == Rollback::type::INSERT)
        Remove(roll.key, roll.timestamp, true);
      else if (roll.op == Rollback::type::REMOVE)
        Insert(roll.key, roll.old_value, roll.timestamp, true);
      else if (roll.op == Rollback::type::MODIFY)
        Modify(roll.key, roll.old_value, roll.timestamp, true);
    }
    return;
  }

 private:
  std::fstream tree_file, leaf_file, rollback;
  std::string tree_filename, leaf_filename, rollback_filename;
  struct Internal {
    bool is_leaf;
    int pos, num;
    int son[internal_size + 1];
    Key key[internal_size + 1];
    Internal() {}
    Internal(int num, int pos, bool is_leaf) {
      memset(son, 0, sizeof(son));
      memset(key, 0, sizeof(key));
      this->num = num;
      this->pos = pos;
      this->is_leaf = is_leaf;
    }
    void Set(int num, int pos, bool is_leaf) {
      this->num = num;
      this->pos = pos;
      this->is_leaf = is_leaf;
    }
  };
  struct Leaf {
    int pos, nxt = 0;
    int num;
    std::pair<Key, Value> val[leaf_size + 1];
    Leaf() {}
    Leaf(int num, int pos, int nxt) {
      memset(val, 0, sizeof(val));
      this->num = num;
      this->pos = pos;
      this->nxt = nxt;
    }
    void Set(int num, int pos, int nxt) {
      this->num = num;
      this->pos = pos;
      this->nxt = nxt;
    }
  };
  struct Rollback {
    enum type { INSERT, REMOVE, MODIFY };
    type op;
    int timestamp;
    Key key;
    Value old_value;
  };
  BufferPool<Internal> internal_pool;
  BufferPool<Leaf> leaf_pool;
  Internal root;
  Leaf leaf;
  Rollback roll;
  int size;
  int rec;
  int last_leaf, last_internal;
  const int INTSIZE = 2 * sizeof(int);

  void Record(typename Rollback::type op, int timestamp, const Key& key,
              const Value& old_value) {
    Rollback ret{op, timestamp, key, old_value};
    rollback.seekp(sizeof(int) + rec * sizeof(Rollback));
    rollback.write(reinterpret_cast<char*>(&ret), sizeof(Rollback));
    rec++;
  }

  void debug(const Internal& internal) {
    Print(internal);
    if (internal.is_leaf) return;
    for (int i = 0; i < internal.num; i++) {
      Internal intt;
      ReadInternal(intt, internal.son[i]);
    }
  }

  void Print(const Internal& internal) {
    for (int i = 0; i < internal.num; i++) std::cout << internal.key[i] << " ";
    std::cout << std::endl;
    for (int i = 0; i < internal.num - 1; i++)
      std::cout << internal.son[i] << " ";
    std::cout << std::endl;
  }

  void Print(const Leaf& leaf) {
    for (int i = 0; i < leaf.num; i++) std::cout << leaf.val[i].first << " ";

    std::cout << std::endl;
    for (int i = 0; i < leaf.num; i++) std::cout << leaf.val[i].second << " ";

    std::cout << std::endl;
  }

  bool InsertIfFatherSplit(const std::pair<Key, Value>& val, Internal& f) {
    if (f.is_leaf) {
      int pos = BinSearchInternalKey(val.first, f);
      ReadLeaf(leaf, f.son[pos]);

      int pos_leaf = BinSearchLeafVal(val, leaf);
      for (int i = leaf.num - 1; i >= pos_leaf; i--)
        leaf.val[i + 1] = leaf.val[i];
      leaf.val[pos_leaf] = val;
      leaf.num++;
      size++;
      if (leaf.num == leaf_size) {
        int m = leaf_size / 2;
        Leaf new_leaf(m, GetLeafIndex(), leaf.nxt);
        for (int i = 0; i < m; i++) new_leaf.val[i] = leaf.val[i + m];
        leaf.nxt = new_leaf.pos;
        leaf.num = m;
        WriteLeaf(leaf);
        WriteLeaf(new_leaf);
        for (int i = f.num - 1; i > pos; i--) f.son[i + 1] = f.son[i];
        for (int i = f.num - 2; i >= pos; i--) f.key[i + 1] = f.key[i];
        f.son[pos + 1] = new_leaf.pos;
        f.key[pos] = leaf.val[m - 1].first;
        f.num++;
        if (f.num == internal_size)
          return true;
        else
          WriteInternal(f);
        return false;
      } else {
        WriteLeaf(leaf);
        return false;
      }
    }
    Internal son;
    int pos = BinSearchInternalKey(val.first, f);
    ReadInternal(son, f.son[pos]);
    if (InsertIfFatherSplit(val, son)) {
      int m = internal_size / 2;
      Internal new_brother(m, GetInternalIndex(), son.is_leaf);
      for (int i = 0; i < m; i++) new_brother.son[i] = son.son[m + i];
      for (int i = 0; i < m - 1; i++) new_brother.key[i] = son.key[m + i];
      son.num = m;
      WriteInternal(son);
      WriteInternal(new_brother);

      for (int i = f.num - 1; i > pos; i--) f.son[i + 1] = f.son[i];
      for (int i = f.num - 2; i >= pos; i--) f.key[i + 1] = f.key[i];
      f.son[pos + 1] = new_brother.pos;
      f.key[pos] = son.key[m - 1];
      f.num++;

      if (f.num == internal_size)
        return true;
      else
        WriteInternal(f);
      return false;
    } else
      return false;
  }
  bool RemoveIfFatherMerge(const Key& key, Internal& f) {
    if (f.is_leaf) {
      int pos = BinSearchInternalKey(key, f);
      ReadLeaf(leaf, f.son[pos]);
      int pos_leaf = BinSearchLeafKey(key, leaf);

      for (int i = pos_leaf; i < leaf.num - 1; i++)
        leaf.val[i] = leaf.val[i + 1];
      leaf.num--;
      size--;
      int m = leaf_size / 2;
      if (leaf.num < m) {
        Leaf sibilings_left, sibilings_right;
        if (pos > 0) {
          ReadLeaf(sibilings_left, f.son[pos - 1]);
          if (sibilings_left.num > m) {
            for (int i = leaf.num - 1; i >= 0; i--)
              leaf.val[i + 1] = leaf.val[i];
            leaf.val[0] = sibilings_left.val[sibilings_left.num - 1];
            sibilings_left.num--;
            leaf.num++;
            f.key[pos - 1] = sibilings_left.val[sibilings_left.num - 1].first;

            WriteLeaf(leaf);
            WriteLeaf(sibilings_left);
            WriteInternal(f);
            return false;
          }
        }
        if (pos < f.num - 1) {
          ReadLeaf(sibilings_right, f.son[pos + 1]);
          if (sibilings_right.num > m) {
            leaf.val[leaf.num] = sibilings_right.val[0];
            for (int i = 0; i < sibilings_right.num - 1; i++)
              sibilings_right.val[i] = sibilings_right.val[i + 1];
            leaf.num++;
            sibilings_right.num--;
            f.key[pos] = leaf.val[leaf.num - 1].first;

            WriteLeaf(leaf);
            WriteLeaf(sibilings_right);
            WriteInternal(f);
            return false;
          }
        }
        if (pos > 0) {
          ReadLeaf(sibilings_left, f.son[pos - 1]);
          for (int i = 0; i < leaf.num; i++)
            sibilings_left.val[sibilings_left.num + i] = leaf.val[i];
          sibilings_left.num += leaf.num;
          sibilings_left.nxt = leaf.nxt;
          WriteLeaf(sibilings_left);
          leaf_pool.Remove(leaf.pos);

          for (int i = pos - 1; i < f.num - 2; i++) f.key[i] = f.key[i + 1];
          for (int i = pos; i < f.num - 1; i++) f.son[i] = f.son[i + 1];
          f.num--;

          if (f.num < m) return true;
          WriteInternal(f);
          return false;
        }
        if (pos < f.num - 1) {
          ReadLeaf(sibilings_right, f.son[pos + 1]);
          for (int i = 0; i < sibilings_right.num; i++)
            leaf.val[leaf.num + i] = sibilings_right.val[i];
          leaf.num += sibilings_right.num;
          leaf.nxt = sibilings_right.nxt;
          WriteLeaf(leaf);
          leaf_pool.Remove(sibilings_right.pos);

          for (int i = pos; i < f.num - 2; i++) f.key[i] = f.key[i + 1];
          for (int i = pos + 1; i < f.num - 1; i++) f.son[i] = f.son[i + 1];
          f.num--;

          if (f.num < m) return true;
          WriteInternal(f);
          return false;
        }
        WriteLeaf(leaf);
      } else
        WriteLeaf(leaf);
      return false;
    }
    Internal son;
    int pos = BinSearchInternalKey(key, f);
    ReadInternal(son, f.son[pos]);
    if (RemoveIfFatherMerge(key, son)) {
      int m = internal_size / 2;
      Internal sibilings_left, sibilings_right;
      if (pos > 0) {
        ReadInternal(sibilings_left, f.son[pos - 1]);
        if (sibilings_left.num > m) {
          for (int i = son.num - 1; i >= 0; i--) son.son[i + 1] = son.son[i];
          for (int i = son.num - 2; i >= 0; i--) son.key[i + 1] = son.key[i];
          son.son[0] = sibilings_left.son[sibilings_left.num - 1];
          son.key[0] = f.key[pos - 1];
          f.key[pos - 1] = sibilings_left.key[sibilings_left.num - 2];
          sibilings_left.num--;
          son.num++;

          WriteInternal(son);
          WriteInternal(sibilings_left);
          WriteInternal(f);
          return false;
        }
      }
      if (pos < f.num - 1) {
        ReadInternal(sibilings_right, f.son[pos + 1]);
        if (sibilings_right.num > m) {
          son.son[son.num] = sibilings_right.son[0];
          son.key[son.num - 1] = f.key[pos];
          f.key[pos] = sibilings_right.key[0];
          // To Be Checked
          for (int i = 0; i < sibilings_right.num - 1; i++)
            sibilings_right.son[i] = sibilings_right.son[i + 1];
          for (int i = 0; i < sibilings_right.num - 2; i++)
            sibilings_right.key[i] = sibilings_right.key[i + 1];
          son.num++;
          sibilings_right.num--;

          WriteInternal(son);
          WriteInternal(sibilings_right);
          WriteInternal(f);
          return false;
        }
      }
      if (pos > 0) {
        ReadInternal(sibilings_left, f.son[pos - 1]);
        for (int i = 0; i < son.num; i++)
          sibilings_left.son[sibilings_left.num + i] = son.son[i];
        sibilings_left.key[sibilings_left.num - 1] = f.key[pos - 1];
        for (int i = 0; i < son.num - 1; i++)
          sibilings_left.key[sibilings_left.num + i] = son.key[i];

        sibilings_left.num += son.num;
        WriteInternal(sibilings_left);
        internal_pool.Remove(son.pos);

        for (int i = pos - 1; i < f.num - 2; i++) f.key[i] = f.key[i + 1];
        for (int i = pos; i < f.num - 1; i++) f.son[i] = f.son[i + 1];
        f.num--;

        if (f.num < m) return true;
        WriteInternal(f);
        return false;
      }
      if (pos < f.num - 1) {
        ReadInternal(sibilings_right, f.son[pos + 1]);
        for (int i = 0; i < sibilings_right.num; i++)
          son.son[son.num + i] = sibilings_right.son[i];
        son.key[son.num - 1] = f.key[pos];
        for (int i = 0; i < sibilings_right.num - 1; i++)
          son.key[son.num + i] = sibilings_right.key[i];
        son.num += sibilings_right.num;
        WriteInternal(son);
        internal_pool.Remove(sibilings_right.pos);

        for (int i = pos; i < f.num - 2; i++) f.key[i] = f.key[i + 1];
        for (int i = pos + 1; i < f.num - 1; i++) f.son[i] = f.son[i + 1];
        f.num--;

        if (f.num < m) return true;
        WriteInternal(f);
        return false;
      }
    }
    return false;
  }
  int BinSearchLeafVal(const std::pair<Key, Value>& val, const Leaf& leaf) {
    // return std::lower_bound(leaf.val, leaf.val + leaf.num, val) -
    // leaf.val;
    int l = -1, r = leaf.num - 1, mid;
    while (l < r) {
      mid = (l + r + 1) >> 1;
      if (val.first <= leaf.val[mid].first)
        r = mid - 1;
      else
        l = mid;
    }
    return l + 1;
  }
  int BinSearchLeafKey(const Key& key, const Leaf& leaf) {
    int l = -1, r = leaf.num - 1, mid;
    while (l < r) {
      mid = (l + r + 1) >> 1;
      if (key <= leaf.val[mid].first)
        r = mid - 1;
      else
        l = mid;
    }
    return l + 1;
  }
  int BinSearchInternalKey(const Key& key, const Internal& internal) {
    int l = -1, r = internal.num - 2, mid;
    while (l < r) {
      mid = (l + r + 1) >> 1;
      if (key <= internal.key[mid])
        r = mid - 1;
      else
        l = mid;
    }
    return l + 1;
  }
  void WriteInternal(const Internal& internal) {
    auto ret = internal_pool.Insert(internal, internal.pos);
    if (ret.first) {
      tree_file.seekp(ret.second.pos * sizeof(Internal) + INTSIZE);
      tree_file.write(reinterpret_cast<char*>(&ret.second), sizeof(Internal));
    }
  }
  void WriteLeaf(const Leaf& leaf) {
    auto ret = leaf_pool.Insert(leaf, leaf.pos);
    if (ret.first) {
      leaf_file.seekp(ret.second.pos * sizeof(Leaf) + INTSIZE);
      leaf_file.write(reinterpret_cast<char*>(&ret.second), sizeof(Leaf));
    }
  }
  void ReadInternal(Internal& internal, int pos) {
    auto ret = internal_pool.Find(pos);
    if (ret.first)
      internal = ret.second;
    else {
      tree_file.seekg(pos * sizeof(Internal) + INTSIZE);
      tree_file.read(reinterpret_cast<char*>(&internal), sizeof(Internal));
    }
  }
  void ReadLeaf(Leaf& leaf, int pos) {
    auto ret = leaf_pool.Find(pos);
    if (ret.first)
      leaf = ret.second;
    else {
      leaf_file.seekg(pos * sizeof(Leaf) + INTSIZE);
      leaf_file.read(reinterpret_cast<char*>(&leaf), sizeof(Leaf));
    }
  }
  int GetInternalIndex() { return ++last_internal; }
  int GetLeafIndex() { return ++last_leaf; }
};
}  // namespace huang
