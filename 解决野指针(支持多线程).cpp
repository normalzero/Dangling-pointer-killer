#define _REF(Ty)\
std::mutex _mtx;\
std::list<Ty**> _refs;\
struct Ref\
{\
    Ref(Ref &ref)\
    {\
        _src = new Ty*;\
        (*_src) = *ref._src;\
        (*_src)->_refs.emplace_back(&(*_src));\
        _spy = &(*_src)->_refs.back();\
    }\
\
    Ref(Ty *k)\
    {\
        _src = new Ty*;\
        (*_src) = k;\
        k->_refs.emplace_back(&(*_src));\
        _spy = &k->_refs.back();\
    }\
\
    ~Ref()\
    {\
        deref();\
        delete _src;\
        _src = NULL;\
    }\
\
    Ty **_src{0};\
    Ty ***_spy{0};\
\
    Ty* get()\
    {\
        Ty *ty = *_src;\
        if (ty)\
        {\
            std::lock_guard<std::mutex> lock(ty->_mtx);\
            return *_src;\
        }\
\
        return 0;\
    }\
    void deref(){ if (*_src) {*_spy = NULL;} *_src = NULL; }\
    Ref& operator=(const Ref &ref) = delete;\
}

template<class Ty>
struct RefTaker
{
    RefTaker(Ty::Ref &ref)
    {
        _take(ref);
    }

    RefTaker& operator=(const Ty::Ref &ref)
    {
        _take(ref);
    }

    ~RefTaker()
    {
        if (_mtr)
            (*_mtr).unlock();
    }

    Ty* _src{ 0 };
    std::mutex *_mtr{ 0 };
    void _take(Ty::Ref &ref)
    {
        Ty *ty = *ref._src;
        if (ty)
        {
            ty->_mtx.lock();
            _src = *ref._src;
            if (_src == NULL)
                ty->_mtx.unlock();
            else
                _mtr = &ty->_mtx;
        }
    }
    Ty* get()
    {
        return _src;
    }
};

class Object
{
public:
    virtual ~Object()
    {
        _for(itr, _refs.begin(), _refs.end())
        {
            std::lock_guard<std::mutex> lock(_mtx);
            if ((*itr) && *(*itr))
                *(*itr) = NULL;
        }
    }

    // 外部非局部引用的指针对象(简称外部使用) 旨在解决原对象与外部引用指针的异步释放产生野指针的问题
    // 外部使用创建时 this存储外部使用的地址_refs 外部使用同时存储 外部使用被this存储的地址_spy
    // 外部使用未释放时 若this未释放 则this拥有在释放时解地址并置空的能力
    // 外部使用释放时 若this未释放 则置空*_spy this不能再访问外部使用的地址(防止异步解地址)
    // this释放时 所有可解地址的外部使用 均被解地址并置空 且_refs自动释放 此时外部使用可根据是否为空判断野指针 且外部使用不再能够访问*_spy
    _REF(Object);
};

std::list<Object::Ref> KList;
int main(int argc, char *argv[])
{
    Object *k1 = new Object();

    KList.emplace_back(k1);
    //KList.emplace_back(k2);
    //KList.emplace_back(k3);

    //KList.begin()->K = NULL;
    Object::Ref ref1(k1);
    RefTaker<Object> taker(ref1);
    Object *obj = taker.get();
    delete k1;
    k1 = NULL;
    KList.erase(KList.begin());
};
