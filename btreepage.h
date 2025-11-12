#ifndef __CBTreePage_H__
#define __CBTreePage_H__

#include <vector>
#include <assert.h>
#include <functional>
#include <utility>
#include <string>

// TODO: #1 Crear una function para agregarla al demo.cpp ( no trivial )
// TODO: #2 Agregarle un Trait (prueba git) ( no trivial )
// TODO: #3 crear un iterator ( no trivial )
//       Sugerencia: Tarea1 cada pagina debe tener un puntero al padre primero ( no trivial )
// TODO: #4 integrarlo al recorrer ( no trivial )

template <typename Trait>
class BTree;

template <typename Trait>
class BTreeIterator;

using namespace std;
enum bt_ErrorCode
{
        bt_ok,
        bt_overflow,
        bt_underflow,
        bt_duplicate,
        bt_nofound,
        bt_rootmerged
};

template <typename Container, typename ObjType, typename Compare>
size_t binary_search(Container &container, size_t first, size_t last, ObjType &object, Compare compare)
{
        if (first >= last)
                return first;
        while (first < last)
        {
                size_t mid = (first + last) / 2;
                if (!compare(object, container[mid]) && !compare(container[mid], object))
                        return mid;
                if (compare(container[mid], object))
                        first = mid + 1;
                else
                        last = mid;
        }
        if (!compare(container[first], object))
                return first;
        return last;
}

// Error al poner size_t
// Posible motivo: El i está disminuyendo
template <typename Container, typename ObjType>
void insert_at(Container &container, ObjType object, int pos)
{
        // TODO: #5 replace int, long by types such as size_t
        size_t size = container.size();
        for (int i = size - 2; i >= pos; i--)
                container[i + 1] = container[i];
        container[pos] = object;
}

template <typename Container>
void remove(Container &container, size_t pos)
{
        size_t size = container.size();
        for (auto i = pos + 1; i < size; i++)
                container[i - 1] = container[i];
}

template <typename keyType, typename ObjIDType>
struct tagObjectInfo
{
        keyType key;
        ObjIDType ObjID;
        size_t UseCounter;
        tagObjectInfo(const keyType &_key, ObjIDType _ObjID)
            : key(_key), ObjID(_ObjID), UseCounter(0) {}
        tagObjectInfo(const tagObjectInfo &objInfo)
            : key(objInfo.key), ObjID(objInfo.ObjID), UseCounter(0) {}
        tagObjectInfo() {}
        operator keyType() const { return key; }
        size_t GetUseCounter() { return UseCounter; }
};

template <typename Trait>
class BTree;
template <typename Trait>
class BTreeIterator;

template <typename Trait>
class CBTreePage //: public SimpleIndex <keyType>
// this is the in-memory version of the CBTreePage
{
        friend class BTree<Trait>;
        friend class BTreeIterator<Trait>;
        friend class BTreeReverseIterator<Trait>;
        // Allow BTree's nested iterators to access internals for traversal
        typedef typename Trait::keyType keyType;
        typedef typename Trait::ObjIDType ObjIDType;
        typedef typename Trait::Compare Compare;

        typedef CBTreePage<Trait> BTPage; // useful shorthand
        typedef tagObjectInfo<keyType, ObjIDType> ObjectInfo;

public:
        CBTreePage(size_t maxKeys, bool unique = true);
        virtual ~CBTreePage();

        CBTreePage(CBTreePage &&other) noexcept
            : m_MinKeys(other.m_MinKeys),
              m_MaxKeys(other.m_MaxKeys),
              m_MaxKeysForChilds(other.m_MaxKeysForChilds),
              m_Unique(other.m_Unique),
              m_isRoot(other.m_isRoot),
              m_Keys(std::move(other.m_Keys)),
              m_SubPages(std::move(other.m_SubPages)),
              m_Parent(std::exchange(other.m_Parent, nullptr)),
              m_Compare(std::move(other.m_Compare)),
              m_KeyCount(std::exchange(other.m_KeyCount, 0))
        {
        }

        CBTreePage &operator=(CBTreePage &&other) noexcept
        {
                if (this != &other)
                {
                        Reset();

                        m_MinKeys = other.m_MinKeys;
                        m_MaxKeys = other.m_MaxKeys;
                        m_MaxKeysForChilds = other.m_MaxKeysForChilds;
                        m_Unique = other.m_Unique;
                        m_isRoot = other.m_isRoot;
                        m_Keys = std::move(other.m_Keys);
                        m_SubPages = std::move(other.m_SubPages);
                        m_Parent = std::exchange(other.m_Parent, nullptr);
                        m_Compare = std::move(other.m_Compare);
                        m_KeyCount = std::exchange(other.m_KeyCount, 0);
                }
                return *this;
        }

        bt_ErrorCode Insert(const keyType &key, const ObjIDType ObjID);
        bt_ErrorCode Remove(const keyType &key, const ObjIDType ObjID);
        bool Search(const keyType &key, ObjIDType &ObjID);
        void Print(ostream &os) const;

        std::ostream &WriteStructure(std::ostream &os) const;
        std::istream &ReadStructure(std::istream &is);

        // TODO: #6 change by Invoke
        // TODO: #7 ForEach must be a template inside this template
        template <typename Func, typename... Args>
        void ForEach(Func &&func, size_t level, Args &&...args);

        template <typename Pred, typename... Args>
        ObjectInfo *FirstThat(Pred &&predicate, size_t level, Args &&...args);

protected:
        // TODO: #9 change by size_t
        size_t m_MinKeys; // minimum number of keys in a node
        size_t m_MaxKeys, // maximum number of keys in a node

            m_MaxKeysForChilds; // just to distinguish the root
        bool m_Unique;
        bool m_isRoot;
        // size_t           NextNode; // address of next node at same level
        // size_t RecAddr; // address of this node in the BTree file
        vector<ObjectInfo> m_Keys;
        vector<BTPage *> m_SubPages;
        BTPage *m_Parent;

        // TODO: #10 size_t
        size_t m_KeyCount;
        Compare m_Compare;
        void Create();
        void Reset();
        void Destroy()
        {
                Reset();
                delete this;
        }
        void clear();

        bool RedistributeWith1Brother(size_t &pos);
        bool RedistributeWith2Brothers(size_t pos);
        void RedistributeR2L(size_t pos);
        void RedistributeL2R(size_t pos);

        bool TreatUnderflow(size_t &pos)
        {
                return RedistributeWith1Brother(pos) || RedistributeWith2Brothers(pos);
        }

        bt_ErrorCode Merge(size_t pos);
        bt_ErrorCode MergeRoot();
        void SplitChild(size_t pos);

        ObjectInfo &GetFirstObjectInfo();

        bool Overflow() { return m_KeyCount > m_MaxKeys; }
        bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
        bool IsFull() { return m_KeyCount >= m_MaxKeys; }

        size_t MinNumberOfKeys() { return 2 * m_MaxKeys / 3.0; }
        size_t GetFreeCells() { return m_MaxKeys - m_KeyCount; }
        size_t &NumberOfKeys() { return m_KeyCount; }
        size_t GetNumberOfKeys() { return m_KeyCount; }
        bool IsRoot() { return m_MaxKeysForChilds != m_MaxKeys; }
        void SetMaxKeysForChilds(size_t orderforchilds)
        {
                m_MaxKeysForChilds = orderforchilds;
        }
        size_t GetFreeCellsOnLeft(size_t pos)
        {
                if (pos > 0) // there is some page on left ?
                        return m_SubPages[pos - 1]->GetFreeCells();
                return 0;
        }
        size_t GetFreeCellsOnRight(size_t pos)
        {
                if (pos < GetNumberOfKeys()) // there is some page on right ?
                        return m_SubPages[pos + 1]->GetFreeCells();
                return 0;
        }

private:
        bool SplitRoot();
        void SplitPageInto3(vector<ObjectInfo> &tmpKeys,
                            vector<BTPage *> &SubPages,
                            BTPage *&pChild1,
                            BTPage *&pChild2,
                            BTPage *&pChild3,
                            ObjectInfo &oi1,
                            ObjectInfo &oi2);
        void MovePage(BTPage *pChildPage, vector<ObjectInfo> &tmpKeys, vector<BTPage *> &tmpSubPages);
};

template <typename Trait>
CBTreePage<Trait>::CBTreePage(size_t maxKeys, bool unique) : m_MaxKeys(maxKeys), m_Unique(unique), m_Parent(nullptr), m_Compare(Compare()), m_KeyCount(0)
{
        Create();
        SetMaxKeysForChilds(m_MaxKeys);
}

template <typename Trait>
CBTreePage<Trait>::~CBTreePage()
{
        Reset();
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Insert(const keyType &key, const ObjIDType ObjID)
{
        size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
        bt_ErrorCode error = bt_ok;

        if (pos < m_KeyCount && (keyType)m_Keys[pos] == key && m_Unique)
                return bt_duplicate;

        if (!m_SubPages[pos])
        {
                ::insert_at(m_Keys, ObjectInfo(key, ObjID), pos);
                m_KeyCount++;
                if (Overflow())
                        return bt_overflow;
                return bt_ok;
        }
        else
        {
                error = m_SubPages[pos]->Insert(key, ObjID);
                if (error == bt_overflow)
                {
                        if (!RedistributeWith1Brother(pos))
                                SplitChild(pos);
                        if (Overflow())
                                return bt_overflow;
                        return bt_ok;
                }
        }
        if (Overflow())
                return bt_overflow;
        return bt_ok;
}

template <typename Trait>
bool CBTreePage<Trait>::RedistributeWith1Brother(size_t &pos)
{
        if (m_SubPages[pos]->Underflow())
        {
                size_t NumberOfKeyOnLeft = 0,
                       NumberOfKeyOnRight = 0;
                if (pos > 0)
                        NumberOfKeyOnLeft = m_SubPages[pos - 1]->NumberOfKeys();
                if (pos < NumberOfKeys())
                        NumberOfKeyOnRight = m_SubPages[pos + 1]->NumberOfKeys();

                if (NumberOfKeyOnLeft > NumberOfKeyOnRight)
                        if (m_SubPages[pos - 1]->NumberOfKeys() > m_SubPages[pos - 1]->MinNumberOfKeys())
                                RedistributeL2R(pos - 1);
                        else
                        {
                                if (pos == NumberOfKeys())
                                        return (--pos, false);

                                else
                                        return false;
                        }
                else
                {
                        if (m_SubPages[pos + 1]->NumberOfKeys() > m_SubPages[pos + 1]->MinNumberOfKeys())
                                RedistributeR2L(pos + 1);
                        else
                        {
                                if (pos == 0)
                                        return (++pos, false);
                                else
                                        return false;
                        }
                }
        }
        else
        {
                size_t FreeCellsOnLeft = GetFreeCellsOnLeft(pos), fcor = GetFreeCellsOnRight(pos);

                if (!FreeCellsOnLeft && !fcor && m_SubPages[pos]->IsFull())
                        return false;
                if (FreeCellsOnLeft > fcor)
                        RedistributeR2L(pos);
                else
                        RedistributeL2R(pos);
        }
        return true;
}

template <typename Trait>
bool CBTreePage<Trait>::RedistributeWith2Brothers(size_t pos)
{
        assert(pos > 0 && pos < NumberOfKeys());
        assert(m_SubPages[pos - 1] != 0 && m_SubPages[pos] != 0 && m_SubPages[pos + 1] != 0);
        assert(m_SubPages[pos - 1]->Underflow() ||
               m_SubPages[pos]->Underflow() ||
               m_SubPages[pos + 1]->Underflow());

        if (m_SubPages[pos - 1]->Underflow())
        {
                RedistributeR2L(pos + 1);
                RedistributeR2L(pos);
                if (m_SubPages[pos - 1]->Underflow())
                        return false;
        }
        else if (m_SubPages[pos + 1]->Underflow())
        {
                RedistributeL2R(pos - 1);
                RedistributeL2R(pos);
                if (m_SubPages[pos + 1]->Underflow())
                        return false;
        }
        else
        {
                RedistributeL2R(pos - 1);
                RedistributeR2L(pos + 1);
                if (m_SubPages[pos]->Underflow())
                        return false;
        }
        return true;
}

template <typename Trait>
void CBTreePage<Trait>::RedistributeR2L(size_t pos)
{
        BTPage *pSource = m_SubPages[pos],
               *pTarget = m_SubPages[pos - 1];

        while (pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
               pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys())
        {
                ::insert_at(pTarget->m_Keys, m_Keys[pos - 1], pTarget->NumberOfKeys()++);
                ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->NumberOfKeys());

                m_Keys[pos - 1] = pSource->m_Keys[0];

                ::remove(pSource->m_Keys, 0);
                ::remove(pSource->m_SubPages, 0);
                pSource->NumberOfKeys()--;
        }
}

template <typename Trait>
void CBTreePage<Trait>::RedistributeL2R(size_t pos)
{
        BTPage *pSource = m_SubPages[pos],
               *pTarget = m_SubPages[pos + 1];
        while (pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
               pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys())
        {
                ::insert_at(pTarget->m_Keys, m_Keys[pos], 0);
                ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->NumberOfKeys()], 0);
                pTarget->NumberOfKeys()++;

                m_Keys[pos] = pSource->m_Keys[pSource->NumberOfKeys() - 1];
                pSource->NumberOfKeys()--;
        }
}

template <typename Trait>
void CBTreePage<Trait>::SplitChild(size_t pos)
{
        BTPage *pChild1 = 0, *pChild2 = 0;
        if (pos > 0)
                if (m_SubPages[pos - 1]->IsFull())
                {
                        pChild1 = m_SubPages[pos - 1];
                        pChild2 = m_SubPages[pos--];
                }
        if (pos < GetNumberOfKeys())
                if (m_SubPages[pos + 1]->IsFull())
                {
                        pChild1 = m_SubPages[pos];
                        pChild2 = m_SubPages[pos + 1];
                }
        size_t nKeys = pChild1->GetNumberOfKeys() + pChild2->GetNumberOfKeys() + 1;

        vector<ObjectInfo> tmpKeys;
        vector<BTPage *> tmpSubPages;

        MovePage(pChild1, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos]);
        MovePage(pChild2, tmpKeys, tmpSubPages);

        BTPage *pChild3 = 0;
        ObjectInfo oi1, oi2;
        SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

        m_Keys[pos] = oi1;
        m_SubPages[pos] = pChild1;
        pChild1->m_Parent = this;

        ::insert_at(m_Keys, oi2, pos + 1);
        ::insert_at(m_SubPages, pChild2, pos + 1);
        NumberOfKeys()++;

        m_SubPages[pos + 2] = pChild3;
        pChild2->m_Parent = this;
        pChild3->m_Parent = this;
}

template <typename Trait>
void CBTreePage<Trait>::SplitPageInto3(vector<ObjectInfo> &tmpKeys,
                                       vector<BTPage *> &tmpSubPages,
                                       BTPage *&pChild1,
                                       BTPage *&pChild2,
                                       BTPage *&pChild3,
                                       ObjectInfo &oi1,
                                       ObjectInfo &oi2)
{
        assert(tmpKeys.size() >= 8);
        assert(tmpSubPages.size() >= 9);
        if (!pChild1)
                pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique);

        pChild1->clear();
        size_t nKeys = (tmpKeys.size() - 2) / 3;
        size_t i = 0;
        for (; i < nKeys; i++)
        {
                pChild1->m_Keys[i] = tmpKeys[i];
                pChild1->m_SubPages[i] = tmpSubPages[i];
                if (tmpSubPages[i])
                        tmpSubPages[i]->m_Parent = pChild1;
                pChild1->NumberOfKeys()++;
        }
        pChild1->m_SubPages[i] = tmpSubPages[i];
        if (tmpSubPages[i])
                tmpSubPages[i]->m_Parent = pChild1;

        oi1 = tmpKeys[i++];

        if (!pChild2)
                pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique);
        pChild2->clear();
        nKeys += (tmpKeys.size() - 2) / 3 + 1;
        size_t j = 0;
        for (; i < nKeys; i++, j++)
        {
                pChild2->m_Keys[j] = tmpKeys[i];
                pChild2->m_SubPages[j] = tmpSubPages[i];
                if (tmpSubPages[i])
                        tmpSubPages[i]->m_Parent = pChild2;
                pChild2->NumberOfKeys()++;
        }
        pChild2->m_SubPages[j] = tmpSubPages[i];
        if (tmpSubPages[i])
                tmpSubPages[i]->m_Parent = pChild2;

        oi2 = tmpKeys[i++];

        if (!pChild3)
                pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique);
        pChild3->clear();
        nKeys = tmpKeys.size();
        for (j = 0; i < nKeys; i++, j++)
        {
                pChild3->m_Keys[j] = tmpKeys[i];
                pChild3->m_SubPages[j] = tmpSubPages[i];
                if (tmpSubPages[i])
                        tmpSubPages[i]->m_Parent = pChild3;
                pChild3->NumberOfKeys()++;
        }
        pChild3->m_SubPages[j] = tmpSubPages[i];
        if (tmpSubPages[i])
                tmpSubPages[i]->m_Parent = pChild3;
}

template <typename Trait>
bool CBTreePage<Trait>::SplitRoot()
{
        BTPage *pChild1 = 0, *pChild2 = 0, *pChild3 = 0;
        ObjectInfo oi1, oi2;
        SplitPageInto3(m_Keys, m_SubPages, pChild1, pChild2, pChild3, oi1, oi2);
        clear();

        m_Keys[0] = oi1;
        m_SubPages[0] = pChild1;
        pChild1->m_Parent = this;
        NumberOfKeys()++;

        m_Keys[1] = oi2;
        m_SubPages[1] = pChild2;
        pChild2->m_Parent = this;
        NumberOfKeys()++;

        m_SubPages[2] = pChild3;
        pChild3->m_Parent = this;
        return true;
}

template <typename Trait>
bool CBTreePage<Trait>::Search(const keyType &key, ObjIDType &ObjID)
{
        size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
        if (pos >= m_KeyCount)
        {
                if (m_SubPages[pos])
                        return m_SubPages[pos]->Search(key, ObjID);
                else
                        return false;
        }
        if (key == m_Keys[pos].key)
        {
                ObjID = m_Keys[pos].ObjID;
                m_Keys[pos].UseCounter++;
                return true;
        }
        if (key < m_Keys[pos].key)
                if (m_SubPages[pos])
                        return m_SubPages[pos]->Search(key, ObjID);
        return false;
}

template <typename Trait>
template <typename Func, typename... Args>
void CBTreePage<Trait>::ForEach(Func &&func, size_t level, Args &&...args)
{
        for (size_t i = 0; i < m_KeyCount; i++)
        {
                if (m_SubPages[i])
                        m_SubPages[i]->ForEach(std::forward<Func>(func), level + 1, std::forward<Args>(args)...);
                func(m_Keys[i], level, std::forward<Args>(args)...);
        }
        if (m_SubPages[m_KeyCount])
                m_SubPages[m_KeyCount]->ForEach(std::forward<Func>(func), level + 1, std::forward<Args>(args)...);
}

template <typename Trait>
template <typename Pred, typename... Args>
typename CBTreePage<Trait>::ObjectInfo *
CBTreePage<Trait>::FirstThat(Pred &&predicate, size_t level, Args &&...args)
{
        ObjectInfo *pTmp;
        for (size_t i = 0; i < m_KeyCount; i++)
        {
                if (m_SubPages[i])
                        if ((pTmp = m_SubPages[i]->FirstThat(std::forward<Pred>(predicate), level + 1, std::forward<Args>(args)...)))
                                return pTmp;
                if (predicate(m_Keys[i], level, std::forward<Args>(args)...))
                        return &m_Keys[i];
        }
        if (m_SubPages[m_KeyCount])
                if ((pTmp = m_SubPages[m_KeyCount]->FirstThat(std::forward<Pred>(predicate), level + 1, std::forward<Args>(args)...)))
                        return pTmp;
        return nullptr;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Remove(const keyType &key, const ObjIDType ObjID)
{
        bt_ErrorCode error = bt_ok;
        size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
        if (pos < NumberOfKeys() && key == m_Keys[pos].key /*&& m_Keys[pos].m_ObjID == ObjID*/) // We found it !
        {
                if (!m_SubPages[pos + 1])
                {
                        ::remove(m_Keys, pos);
                        NumberOfKeys()--;
                        if (Underflow())
                                return bt_underflow;
                        return bt_ok;
                }

                {
                        ObjectInfo &rFirstFromRight = m_SubPages[pos + 1]->GetFirstObjectInfo();
                        swap(m_Keys[pos], rFirstFromRight);
                        error = m_SubPages[++pos]->Remove(key, ObjID);
                }
        }
        else if (pos == NumberOfKeys())
                error = m_SubPages[pos]->Remove(key, ObjID);
        else if (key <= m_Keys[pos].key)
        {
                if (m_SubPages[pos])
                        error = m_SubPages[pos]->Remove(key, ObjID);
                else
                        return bt_nofound;
        }
        if (error == bt_underflow)
        {
                if (TreatUnderflow(pos))
                        return bt_ok;
                if (IsRoot() && NumberOfKeys() == 2)
                        return MergeRoot();
                return Merge(pos);
        }
        if (error == bt_nofound)
                return bt_nofound;
        return bt_ok;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Merge(size_t pos)
{
        assert(m_SubPages[pos - 1]->NumberOfKeys() +
                   m_SubPages[pos]->NumberOfKeys() +
                   m_SubPages[pos + 1]->NumberOfKeys() ==
               3 * m_SubPages[pos]->MinNumberOfKeys() - 1);

        vector<ObjectInfo> tmpKeys;
        vector<BTPage *> tmpSubPages;

        BTPage *pChild1 = m_SubPages[pos - 1],
               *pChild2 = m_SubPages[pos],
               *pChild3 = m_SubPages[pos + 1];
        MovePage(pChild1, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos - 1]);
        MovePage(pChild2, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos]);
        MovePage(pChild3, tmpKeys, tmpSubPages);
        pChild3->Destroy();
        ;

        size_t nKeys = pChild1->GetFreeCells();
        size_t i = 0;
        for (; i < nKeys; i++)
        {
                pChild1->m_Keys[i] = tmpKeys[i];
                pChild1->m_SubPages[i] = tmpSubPages[i];
                if (tmpSubPages[i])
                        tmpSubPages[i]->m_Parent = pChild1;
                pChild1->NumberOfKeys()++;
        }
        pChild1->m_SubPages[i] = tmpSubPages[i];
        if (tmpSubPages[i])
                tmpSubPages[i]->m_Parent = pChild1;

        m_Keys[pos - 1] = tmpKeys[i];
        m_SubPages[pos - 1] = pChild1;
        pChild1->m_Parent = this;

        ::remove(m_Keys, pos);
        ::remove(m_SubPages, pos);
        NumberOfKeys()--;

        nKeys = pChild2->GetFreeCells();

        size_t j = ++i;
        for (i = 0; i < nKeys; i++, j++)
        {
                pChild2->m_Keys[i] = tmpKeys[j];
                pChild2->m_SubPages[i] = tmpSubPages[j];
                if (tmpSubPages[j])
                        tmpSubPages[j]->m_Parent = pChild2;
                pChild2->NumberOfKeys()++;
        }
        pChild2->m_SubPages[i] = tmpSubPages[j];
        if (tmpSubPages[j])
                tmpSubPages[j]->m_Parent = pChild2;
        m_SubPages[pos] = pChild2;
        pChild2->m_Parent = this;

        if (Underflow())
                return bt_underflow;
        return bt_ok;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::MergeRoot()
{
        size_t pos = 1;
        assert(m_SubPages[pos - 1]->NumberOfKeys() +
                   m_SubPages[pos]->NumberOfKeys() +
                   m_SubPages[pos + 1]->NumberOfKeys() ==
               3 * m_SubPages[pos]->MinNumberOfKeys() - 1);

        BTPage *pChild1 = m_SubPages[pos - 1], *pChild2 = m_SubPages[pos], *pChild3 = m_SubPages[pos + 1];
        size_t nKeys = pChild1->NumberOfKeys() + pChild2->NumberOfKeys() + pChild3->NumberOfKeys() + 2;

        vector<ObjectInfo> tmpKeys;
        vector<BTPage *> tmpSubPages;

        MovePage(pChild1, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos - 1]);
        MovePage(pChild2, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos]);
        MovePage(pChild3, tmpKeys, tmpSubPages);

        clear();
        size_t i = 0;
        for (; i < nKeys; i++)
        {
                m_Keys[i] = tmpKeys[i];
                m_SubPages[i] = tmpSubPages[i];
                if (tmpSubPages[i])
                        tmpSubPages[i]->m_Parent = this;
                NumberOfKeys()++;
        }
        m_SubPages[i] = tmpSubPages[i];
        if (tmpSubPages[i])
                tmpSubPages[i]->m_Parent = this;

        pChild1->Destroy();
        pChild2->Destroy();
        pChild3->Destroy();

        return bt_rootmerged;
}

template <typename Trait>
typename CBTreePage<Trait>::ObjectInfo &
CBTreePage<Trait>::GetFirstObjectInfo()
{
        if (m_SubPages[0])
                return m_SubPages[0]->GetFirstObjectInfo();
        return m_Keys[0];
}

template <typename keyType, typename ObjIDType>
void Print(tagObjectInfo<keyType, ObjIDType> &info, size_t level, void *pExtra)
{
        ostream &os = *(ostream *)pExtra;
        for (size_t i = 0; i < level; i++)
                os << "\t";
        os << info.key << "->" << info.ObjID << "\n";
}

template <typename Trait>
void CBTreePage<Trait>::Print(ostream &os) const
{
        const_cast<CBTreePage<Trait> *>(this)->ForEach([&os](ObjectInfo &info, size_t level)
                                                       {
               for(size_t i = 0; i < level; i++)
                       os << "\t";
               os << info.key << "->" << info.ObjID << "\n"; }, 0);
}

template <typename Trait>
void CBTreePage<Trait>::Create()
{
        Reset();
        m_Keys.resize(m_MaxKeys + 1);
        m_SubPages.resize(m_MaxKeys + 2, NULL);
        m_KeyCount = 0;
        m_MinKeys = 2 * m_MaxKeys / 3;
}

template <typename Trait>
void CBTreePage<Trait>::Reset()
{
        for (size_t i = 0; i < m_KeyCount; i++)
                delete m_SubPages[i];
        clear();
}

template <typename Trait>
void CBTreePage<Trait>::clear()
{
        m_KeyCount = 0;
}

template <typename Trait>
CBTreePage<Trait> *CreateBTreeNode(size_t maxKeys, bool unique)
{
        return new CBTreePage<Trait>(maxKeys, unique);
}

template <typename Trait>
void CBTreePage<Trait>::MovePage(BTPage *pChildPage, vector<ObjectInfo> &tmpKeys, vector<BTPage *> &tmpSubPages)
{
        size_t nKeys = pChildPage->GetNumberOfKeys();
        size_t i = 0;
        for (i = 0; i < nKeys; i++)
        {
                tmpKeys.push_back(pChildPage->m_Keys[i]);
                tmpSubPages.push_back(pChildPage->m_SubPages[i]);
        }
        tmpSubPages.push_back(pChildPage->m_SubPages[i]);
        pChildPage->clear();
}

template <typename Trait>
std::ostream &CBTreePage<Trait>::WriteStructure(std::ostream &os) const
{
        os << m_KeyCount << "\n";

        for (size_t i = 0; i < m_KeyCount; ++i)
        {
                os << m_Keys[i].key << " " << m_Keys[i].ObjID << "\n";
        }

        for (size_t i = 0; i <= m_KeyCount; ++i)
        {
                if (m_SubPages[i])
                {
                        os << "1\n";
                        m_SubPages[i]->WriteStructure(os);
                }
                else
                {
                        os << "0\n";
                }
        }

        return os;
}

template <typename Trait>
std::istream &CBTreePage<Trait>::ReadStructure(std::istream &is)
{
        size_t keyCount;
        is >> keyCount;

        for (size_t i = 0; i < keyCount; ++i)
        {
                typename Trait::keyType key;
                typename Trait::ObjIDType objID;
                is >> key >> objID;

                m_Keys[i] = ObjectInfo(key, objID);
        }
        m_KeyCount = keyCount;

        for (size_t i = 0; i <= keyCount; ++i)
        {
                int hasChild;
                is >> hasChild;

                if (hasChild)
                {
                        m_SubPages[i] = new BTPage(m_MaxKeysForChilds * 2 + 1, m_Unique);
                        m_SubPages[i]->SetMaxKeysForChilds(m_MaxKeysForChilds);
                        m_SubPages[i]->m_Parent = this;
                        m_SubPages[i]->ReadStructure(is);
                }
                else
                {
                        m_SubPages[i] = nullptr;
                }
        }

        return is;
}

#endif