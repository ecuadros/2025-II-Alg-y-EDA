#pragma once
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <initializer_list>
#include <string>

template <typename T, int D>
struct Point
{
    T coords[D]{};

    Point()
    {
        for (int i = 0; i < D; ++i)
            coords[i] = T{};
    }

    Point(std::initializer_list<T> l)
    {
        int i = 0;
        for (auto v : l)
        {
            if (i < D)
                coords[i++] = v;
        }
        for (; i < D; ++i)
            coords[i] = T{};
    }

    friend std::ostream& operator<<(std::ostream& os, const Point& p)
    {
        os << "(";
        for (int i = 0; i < D; ++i)
            os << p.coords[i] << (i < D - 1 ? "," : "");
        os << ")";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Point& p)
    {
        char c;
        is >> c;
        for (int i = 0; i < D; ++i)
        {
            is >> p.coords[i];
            if (i < D - 1)
                is >> c;
        }
        is >> c;
        return is;
    }
};

template <typename T, int D>
struct Rect
{
    Point<T, D> min_pt;
    Point<T, D> max_pt;

    Rect() = default;
    Rect(Point<T, D> a, Point<T, D> b) : min_pt(a), max_pt(b) {}

    double area() const
    {
        double a = 1.0;
        for (int i = 0; i < D; ++i)
            a *= double(max_pt.coords[i] - min_pt.coords[i]);
        return a;
    }

    bool overlaps(const Rect& other) const
    {
        for (int i = 0; i < D; ++i)
        {
            if (min_pt.coords[i] > other.max_pt.coords[i] || max_pt.coords[i] < other.min_pt.coords[i])
                return false;
        }
        return true;
    }

    double expansion_needed(const Rect& other) const
    {
        double expanded_area = 1.0;
        for (int i = 0; i < D; ++i)
        {
            T mn = std::min(min_pt.coords[i], other.min_pt.coords[i]);
            T mx = std::max(max_pt.coords[i], other.max_pt.coords[i]);
            expanded_area *= double(mx - mn);
        }
        return expanded_area - area();
    }

    void expand(const Rect& other)
    {
        for (int i = 0; i < D; ++i)
        {
            min_pt.coords[i] = std::min(min_pt.coords[i], other.min_pt.coords[i]);
            max_pt.coords[i] = std::max(max_pt.coords[i], other.max_pt.coords[i]);
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Rect& r)
    {
        os << "[" << r.min_pt << ";" << r.max_pt << "]";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Rect& r)
    {
        char c;
        is >> c;
        is >> r.min_pt >> c;
        is >> r.max_pt;
        is >> c;
        return is;
    }

    friend bool operator==(const Rect& a, const Rect& b)
    {
        for (int i = 0; i < D; ++i)
        {
            if (a.min_pt.coords[i] != b.min_pt.coords[i] || a.max_pt.coords[i] != b.max_pt.coords[i])
                return false;
        }
        return true;
    }
};

template <typename Traits>
class CRTreeNode
{
public:
    using coordinate_type = typename Traits::CoordinateType;
    using data_type       = typename Traits::DataType;
    static constexpr int dim = Traits::D;

    using rect_type = Rect<coordinate_type, dim>;
    using node_type = CRTreeNode<Traits>;

    struct Entry
    {
        rect_type mbr;
        node_type* p_child = nullptr;
        data_type data{};
    };

    explicit CRTreeNode(bool leaf = true) : is_leaf_(leaf) {}

    ~CRTreeNode()
    {
        if (!is_leaf_)
        {
            for (auto& e : entries_)
                delete e.p_child;
        }
    }

    bool   is_leaf() const { return is_leaf_; }
    size_t count()   const { return entries_.size(); }

    rect_type get_mbr() const
    {
        if (entries_.empty())
            return rect_type();
        rect_type res = entries_[0].mbr;
        for (size_t i = 1; i < entries_.size(); ++i)
            res.expand(entries_[i].mbr);
        return res;
    }

private:
    bool is_leaf_ = true;

    template <typename>
    friend class CRTree;

    std::vector<Entry> entries_;
    node_type* parent_ = nullptr;
};

template <typename Traits>
class CRTree
{
public:
    using coordinate_type = typename Traits::CoordinateType;
    using data_type       = typename Traits::DataType;
    static constexpr int dim = Traits::D;

    using rect_type = Rect<coordinate_type, dim>;
    using node_type = CRTreeNode<Traits>;
    using entry_type = typename node_type::Entry;

    CRTree() : root_(new node_type(true)) {}
    ~CRTree() { delete root_; }

    void insert(const rect_type& r, const data_type& data, bool increase_size = true)
    {
        entry_type e;
        e.mbr = r;
        e.data = data;
        e.p_child = nullptr;

        insert_entry(root_, e);
        size_ += increase_size;
    }

    std::vector<data_type> search(const rect_type& query) const
    {
        std::vector<data_type> results;
        search_rec(root_, query, results);
        return results;
    }

    bool remove(const rect_type& r, const data_type& data)
    {
        node_type* leaf = find_leaf(root_, r, data);
        if (!leaf)
            return false;

        auto& v = leaf->entries_;
        auto it = std::find_if(v.begin(), v.end(), [&](const entry_type& e)
        {
            return e.data == data && e.mbr == r;
        });
        if (it == v.end())
            return false;

        v.erase(it);
        condense_tree(leaf);

        if (!root_->is_leaf() && root_->count() == 1)
        {
            node_type* new_root = root_->entries_[0].p_child;
            root_->entries_.clear();
            delete root_;
            root_ = new_root;
            root_->parent_ = nullptr;
        }
        if (!root_->is_leaf() && root_->count() == 0)
        {
            delete root_;
            root_ = new node_type(true);
        }

        --size_;
        return true;
    }

    void write(std::ostream& os) const { write_rec(root_, os); }

    void read(std::istream& is)
    {
        delete root_;
        root_ = read_rec(is, nullptr);
        if (!root_)
            root_ = new node_type(true);
    }

    bool write_to_file(const std::string& path) const
    {
        std::ofstream f(path);
        if (!f)
            return false;
        write(f);
        return true;
    }

    bool read_from_file(const std::string& path)
    {
        std::ifstream f(path);
        if (!f)
            return false;
        read(f);
        return true;
    }

private:
    node_type* root_ = nullptr;
    size_t size_ = 0;

private:
    void insert_entry(node_type* node, entry_type& e)
    {
        if (node->is_leaf())
        {
            node->entries_.push_back(e);
            if (node->entries_.size() > Traits::M)
                split_node(node);
            return;
        }

        node_type* child = choose_subtree(node, e.mbr);
        insert_entry(child, e);
        adjust_tree(node);
    }

    node_type* choose_subtree(node_type* node, const rect_type& r)
    {
        double min_enl = 1e300;
        size_t best = 0;

        for (size_t i = 0; i < node->entries_.size(); ++i)
        {
            double enl = node->entries_[i].mbr.expansion_needed(r);
            if (enl < min_enl)
            {
                min_enl = enl;
                best = i;
            }
            else if (enl == min_enl)
            {
                if (node->entries_[i].mbr.area() < node->entries_[best].mbr.area())
                    best = i;
            }
        }
        return node->entries_[best].p_child;
    }

    void adjust_tree(node_type* node)
    {
        for (auto& e : node->entries_)
        {
            if (e.p_child)
                e.mbr = e.p_child->get_mbr();
        }
    }

    void split_node(node_type* node)
    {
        node_type* new_node = new node_type(node->is_leaf());
        new_node->parent_ = node->parent_;

        int seed_1 = 0, seed_2 = 1;
        pick_seeds(node->entries_, seed_1, seed_2);

        std::vector<entry_type> group_1, group_2;
        group_1.push_back(node->entries_[seed_1]);
        group_2.push_back(node->entries_[seed_2]);

        if (seed_1 > seed_2)
            std::swap(seed_1, seed_2);

        std::vector<entry_type> remaining;
        remaining.reserve(node->entries_.size());
        for (size_t i = 0; i < node->entries_.size(); ++i)
            if ((int)i != seed_1 && (int)i != seed_2)
                remaining.push_back(node->entries_[i]);

        rect_type mbr_1 = group_1[0].mbr;
        rect_type mbr_2 = group_2[0].mbr;

        for (const auto& entry : remaining)
        {
            if (Traits::M + 1 - (group_1.size() + group_2.size()) == Traits::m - group_1.size())
            {
                group_1.push_back(entry);
                continue;
            }
            if (Traits::M + 1 - (group_1.size() + group_2.size()) == Traits::m - group_2.size())
            {
                group_2.push_back(entry);
                continue;
            }

            double d1 = mbr_1.expansion_needed(entry.mbr);
            double d2 = mbr_2.expansion_needed(entry.mbr);

            if (d1 < d2)
            {
                group_1.push_back(entry);
                mbr_1.expand(entry.mbr);
            }
            else
            {
                group_2.push_back(entry);
                mbr_2.expand(entry.mbr);
            }
        }

        node->entries_ = std::move(group_1);
        new_node->entries_ = std::move(group_2);

        if (!node->is_leaf())
        {
            for (auto& e : node->entries_)
                if (e.p_child)
                    e.p_child->parent_ = node;
            for (auto& e : new_node->entries_)
                if (e.p_child)
                    e.p_child->parent_ = new_node;
        }

        if (!node->parent_)
        {
            node_type* new_root = new node_type(false);
            entry_type e1;
            e1.p_child = node;
            e1.mbr = node->get_mbr();
            entry_type e2;
            e2.p_child = new_node;
            e2.mbr = new_node->get_mbr();
            new_root->entries_.push_back(e1);
            new_root->entries_.push_back(e2);
            node->parent_ = new_root;
            new_node->parent_ = new_root;
            root_ = new_root;
            return;
        }

        node_type* parent = node->parent_;
        for (auto& pe : parent->entries_)
        {
            if (pe.p_child == node)
            {
                pe.mbr = node->get_mbr();
                break;
            }
        }
        entry_type e_new;
        e_new.p_child = new_node;
        e_new.mbr = new_node->get_mbr();
        parent->entries_.push_back(e_new);

        if (parent->entries_.size() > Traits::M)
            split_node(parent);
    }

    void pick_seeds(const std::vector<entry_type>& entries, int& seed_1, int& seed_2)
    {
        seed_1 = 0;
        seed_2 = 1;
        if (entries.size() < 2)
            return;

        rect_type total = entries[0].mbr;
        for (size_t i = 1; i < entries.size(); ++i)
            total.expand(entries[i].mbr);

        double best = -1.0;

        for (int d = 0; d < dim; ++d)
        {
            int hi_low_idx = -1, lo_high_idx = -1;
            coordinate_type hi_low = coordinate_type{};
            coordinate_type lo_high = coordinate_type{};

            for (size_t i = 0; i < entries.size(); ++i)
            {
                if (hi_low_idx == -1 || entries[i].mbr.min_pt.coords[d] > hi_low)
                {
                    hi_low = entries[i].mbr.min_pt.coords[d];
                    hi_low_idx = (int)i;
                }
                if (lo_high_idx == -1 || entries[i].mbr.max_pt.coords[d] < lo_high)
                {
                    lo_high = entries[i].mbr.max_pt.coords[d];
                    lo_high_idx = (int)i;
                }
            }

            if (hi_low_idx != -1 && lo_high_idx != -1 && hi_low_idx != lo_high_idx)
            {
                double sep = std::abs(double(hi_low) - double(lo_high));
                double width = std::abs(double(total.max_pt.coords[d]) - double(total.min_pt.coords[d]));
                double norm = (width > 1e-9) ? (sep / width) : sep;

                if (norm > best)
                {
                    best = norm;
                    seed_1 = hi_low_idx;
                    seed_2 = lo_high_idx;
                }
            }
        }

        if (seed_1 == seed_2)
            seed_2 = (seed_1 + 1) % (int)entries.size();
    }

    static void search_rec(node_type* node, const rect_type& query, std::vector<data_type>& out)
    {
        if (!node)
            return;
        for (auto& e : node->entries_)
        {
            if (!e.mbr.overlaps(query))
                continue;
            if (node->is_leaf())
                out.push_back(e.data);
            else
                search_rec(e.p_child, query, out);
        }
    }

    node_type* find_leaf(node_type* node, const rect_type& r, const data_type& data)
    {
        if (!node)
            return nullptr;
        if (node->is_leaf())
        {
            for (auto& e : node->entries_)
                if (e.data == data)
                    return node;
            return nullptr;
        }
        for (auto& e : node->entries_)
        {
            if (e.mbr.overlaps(r) || e.mbr.expansion_needed(r) == 0.0)
            {
                node_type* res = find_leaf(e.p_child, r, data);
                if (res)
                    return res;
            }
        }
        return nullptr;
    }

    void condense_tree(node_type* node)
    {
        node_type* p = node;
        std::vector<node_type*> orphans;

        while (p != root_)
        {
            node_type* parent = p->parent_;

            if (p->count() < (size_t)Traits::m)
            {
                auto& pe = parent->entries_;
                pe.erase(std::remove_if(pe.begin(), pe.end(),
                                        [&](const entry_type& e)
                                        { return e.p_child == p; }),
                         pe.end());
                orphans.push_back(p);
            }
            else
            {
                for (auto& e : parent->entries_)
                {
                    if (e.p_child == p)
                    {
                        e.mbr = p->get_mbr();
                        break;
                    }
                }
            }
            p = parent;
        }

        for (node_type* o : orphans)
        {
            reinsert_node_entries(o);
            o->entries_.clear();
            delete o;
        }
    }

    void reinsert_node_entries(node_type* node)
    {
        if (node->is_leaf())
        {
            for (const auto& e : node->entries_)
                insert(e.mbr, e.data, false);
            return;
        }
        for (const auto& e : node->entries_)
        {
            reinsert_node_entries(e.p_child);
            delete e.p_child;
        }
    }

    void write_rec(node_type* node, std::ostream& os) const
    {
        if (!node)
            return;
        os << node->is_leaf() << " " << node->count() << "\n";
        for (size_t i = 0; i < node->count(); ++i)
        {
            os << node->entries_[i].mbr << " ";
            if (node->is_leaf())
            {
                os << node->entries_[i].data << "\n";
            }
            else
            {
                os << "\n";
                write_rec(node->entries_[i].p_child, os);
            }
        }
    }

    node_type* read_rec(std::istream& is, node_type* parent)
    {
        bool is_leaf;
        size_t count;
        if (!(is >> is_leaf >> count))
            return nullptr;

        node_type* node = new node_type(is_leaf);
        node->parent_ = parent;

        for (size_t i = 0; i < count; ++i)
        {
            entry_type e;
            is >> e.mbr;
            if (is_leaf)
            {
                is >> e.data;
                e.p_child = nullptr;
            }
            else
            {
                e.p_child = read_rec(is, node);
            }
            node->entries_.push_back(e);
        }
        return node;
    }
};

struct RTreeTraits2D
{
    using CoordinateType = float;
    using DataType = int;
    static constexpr int D = 2;
    static constexpr int M = 8;
    static constexpr int m = 4;
};
