// src.hpp - Pokedex implementation for ACMOJ 1277
#include <bits/stdc++.h>
using namespace std;

class BasicException {
protected:
    string msg;
public:
    explicit BasicException(const char *_message) : msg(_message ? _message : "") {}
    virtual const char *what() const { return msg.c_str(); }
    virtual ~BasicException() {}
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const char *_message): BasicException(_message) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const char *_message): BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    vector<string> types;
};

namespace detail {
static inline bool isAlphaStr(const char *s) {
    if (s == nullptr || *s == '\0') return false;
    for (const char *p = s; *p; ++p) {
        char c = *p;
        bool lower = (c >= 'a' && c <= 'z');
        bool upper = (c >= 'A' && c <= 'Z');
        if (!(lower || upper)) return false;
    }
    return true;
}

static inline vector<string> splitTypes(const char *types){
    vector<string> res;
    if (types == nullptr || *types == '\0') return res;
    string t(types);
    size_t i=0;
    while (i<=t.size()) {
        size_t j=t.find('#', i);
        string part = t.substr(i, j==string::npos? string::npos: j-i);
        if (part.size() > 0) res.push_back(part); else res.push_back("");
        if (j==string::npos) break;
        i=j+1;
    }
    return res;
}

static inline bool allAlpha(const vector<string>& v){
    if (v.empty()) return false;
    for (auto &s: v){
        if (s.size() == 0) return false;
        for (char c: s){
            bool lower = (c >= 'a' && c <= 'z');
            bool upper = (c >= 'A' && c <= 'Z');
            if (!(lower || upper)) return false;
        }
    }
    return true;
}

static inline bool isValidTypeName(const string& s){
    static const unordered_set<string> allowed = {
        "water","fire","dragon","ground","electric","flying","grass"
    };
    return allowed.count(s) > 0;
}

static inline float effOne(const string& atk, const string& def){
    // 7-type chart (subset of standard Pokemon)
    if (atk=="water") {
        if (def=="fire" || def=="ground") return 2.0f;
        if (def=="water" || def=="grass" || def=="dragon") return 0.5f;
        return 1.0f;
    }
    if (atk=="fire") {
        if (def=="grass") return 2.0f;
        if (def=="fire" || def=="water" || def=="dragon") return 0.5f;
        return 1.0f;
    }
    if (atk=="grass") {
        if (def=="water" || def=="ground") return 2.0f;
        if (def=="fire" || def=="grass" || def=="dragon" || def=="flying") return 0.5f;
        return 1.0f;
    }
    if (atk=="electric") {
        if (def=="water" || def=="flying") return 2.0f;
        if (def=="ground") return 0.0f;
        if (def=="electric" || def=="grass" || def=="dragon") return 0.5f;
        return 1.0f;
    }
    if (atk=="ground") {
        if (def=="fire" || def=="electric") return 2.0f;
        if (def=="flying") return 0.0f;
        if (def=="grass") return 0.5f;
        return 1.0f;
    }
    if (atk=="flying") {
        if (def=="grass") return 2.0f;
        if (def=="electric") return 0.5f;
        return 1.0f;
    }
    if (atk=="dragon") {
        if (def=="dragon") return 2.0f;
        return 1.0f;
    }
    return 1.0f;
}

static inline float attackMulti(const vector<string>& atkTypes, const vector<string>& defTypes){
    float best=0.0f;
    for (auto &atk: atkTypes){
        float mul=1.0f;
        for (auto &df: defTypes){
            mul *= effOne(atk, df);
            if (mul==0.0f) break;
        }
        if (mul>best) best=mul;
    }
    return best;
}

} // namespace detail

class Pokedex {
private:
    string filename;
    map<int, Pokemon> byId; // sorted by id ascending
    unordered_map<string,int> nameToId;

    static string trim(const string& s){
        size_t a=s.find_first_not_of(" \t\r\n");
        size_t b=s.find_last_not_of(" \t\r\n");
        if (a==string::npos) return "";
        return s.substr(a,b-a+1);
    }

    void load() {
        byId.clear(); nameToId.clear();
        if (filename.empty()) return;
        ifstream fin(filename);
        if (!fin.good()) {
            ofstream create(filename);
            return;
        }
        string line;
        while (getline(fin, line)) {
            line = trim(line);
            if (line.empty()) continue;
            int id; string name, types;
            {
                stringstream ss(line);
                string sid;
                if (!getline(ss, sid, '\t')) continue;
                if (!getline(ss, name, '\t')) continue;
                if (!getline(ss, types)) types.clear();
                try { id = stoi(sid); } catch(...) { continue; }
            }
            Pokemon pm; pm.id=id;
            memset(pm.name, 0, sizeof(pm.name));
            strncpy(pm.name, name.c_str(), sizeof(pm.name)-1);
            pm.types = detail::splitTypes(types.c_str());
            byId[id]=pm;
            nameToId[string(pm.name)] = id;
        }
    }

    void save() const {
        if (filename.empty()) return;
        ofstream fout(filename, ios::trunc);
        for (auto &kv: byId) {
            const Pokemon &pm = kv.second;
            fout << pm.id << '\t' << pm.name << '\t';
            for (size_t i=0;i<pm.types.size();++i){ if (i) fout << '#'; fout << pm.types[i]; }
            fout << '\n';
        }
    }

    static string firstInvalidType(const vector<string>& v){
        for (auto &t: v){ if (!detail::isValidTypeName(t)) return t; }
        return "";
    }

public:
    explicit Pokedex(const char *_fileName) {
        filename = _fileName ? string(_fileName) : string();
        load();
    }

    ~Pokedex() {
        try { save(); } catch(...) {}
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        if (!detail::isAlphaStr(name)) {
            string sname = name? string(name): string("");
            string msg = string("Argument Error: PM Name Invalid (")+ sname + ")";
            throw ArgumentException(msg.c_str());
        }
        if (id <= 0) {
            string msg = string("Argument Error: PM ID Invalid (") + to_string(id) + ")";
            throw ArgumentException(msg.c_str());
        }
        vector<string> tv = detail::splitTypes(types);
        if (!detail::allAlpha(tv)) {
            string bad;
            if (tv.empty()) bad = "";
            else {
                for (auto &t: tv){ if (!detail::isAlphaStr(t.c_str())) { bad=t; break; } }
            }
            string msg = string("Argument Error: PM Type Invalid (") + bad + ")";
            throw ArgumentException(msg.c_str());
        }
        string bad = firstInvalidType(tv);
        if (bad.size() > 0) {
            string msg = string("Argument Error: PM Type Invalid (") + bad + ")";
            throw ArgumentException(msg.c_str());
        }
        Pokemon pm; pm.id = id;
        memset(pm.name, 0, sizeof(pm.name));
        strncpy(pm.name, name, sizeof(pm.name)-1);
        pm.types = tv;
        // enforce unique id and unique name
        if (byId.find(id) != byId.end()) return false;
        if (nameToId.find(string(pm.name)) != nameToId.end()) return false;
        bool inserted = byId.emplace(id, pm).second;
        if (!inserted) return false;
        nameToId[string(pm.name)] = id;
        return true;
    }

    bool pokeDel(int id) {
        auto it = byId.find(id);
        if (it==byId.end()) return false;
        string nm = it->second.name;
        byId.erase(it);
        nameToId.erase(nm);
        return true;
    }

    string pokeFind(int id) const {
        auto it = byId.find(id);
        if (it==byId.end()) return string("None");
        return string(it->second.name);
    }

    string typeFind(const char *types) const {
        vector<string> q = detail::splitTypes(types);
        if (!detail::allAlpha(q)) {
            string bad;
            if (q.empty()) bad = ""; else { for (auto &t: q){ if (!detail::isAlphaStr(t.c_str())) { bad=t; break; } } }
            string msg = string("Argument Error: PM Type Invalid (") + bad + ")";
            throw ArgumentException(msg.c_str());
        }
        string bad = firstInvalidType(q);
        if (bad.size() > 0) {
            string msg = string("Argument Error: PM Type Invalid (") + bad + ")";
            throw ArgumentException(msg.c_str());
        }
        vector<const Pokemon*> res;
        for (auto &kv: byId){
            const Pokemon &pm = kv.second;
            bool ok = true;
            for (auto &t: q){ if (find(pm.types.begin(), pm.types.end(), t)==pm.types.end()){ ok=false; break; } }
            if (ok) res.push_back(&kv.second);
        }
        if (res.empty()) return string("None");
        string out = to_string(res.size());
        for (auto *p: res){ out.push_back('\n'); out += p->name; }
        return out;
    }

    float attack(const char *type, int id) const {
        auto it = byId.find(id);
        if (it==byId.end()) return -1.0f;
        string t = (type ? string(type) : string());
        vector<string> atk;
        atk.push_back(t);
        return detail::attackMulti(atk, it->second.types);
    }

    int catchTry() const {
        if (byId.empty()) return 0;
        auto it = byId.begin();
        unordered_set<int> owned; owned.insert(it->first);
        vector<string> ownedTypes;
        auto appendTypes=[&](const vector<string>& ts){
            for (auto &x: ts){ if (find(ownedTypes.begin(), ownedTypes.end(), x)==ownedTypes.end()) ownedTypes.push_back(x); }
        };
        appendTypes(it->second.types);
        bool changed=true;
        while (changed){
            changed=false;
            for (auto &kv: byId){
                int pid = kv.first; if (owned.count(pid)) continue;
                float mul = detail::attackMulti(ownedTypes, kv.second.types);
                if (mul >= 2.0f - 1e-6f){
                    owned.insert(pid);
                    appendTypes(kv.second.types);
                    changed=true;
                }
            }
        }
        return (int)owned.size();
    }

    struct iterator {
        const Pokedex *dex;
        map<int,Pokemon>::const_iterator it;

        iterator(): dex(nullptr) {}
        iterator(const Pokedex* d, map<int,Pokemon>::const_iterator i): dex(d), it(i) {}

        iterator &operator++() {
            if (dex == nullptr) throw IteratorException("Iterator Error: Invalid Iterator");
            if (it==dex->byId.end()) throw IteratorException("Iterator Error: Out Of Bound");
            ++it; return *this;
        }
        iterator &operator--() {
            if (dex == nullptr) throw IteratorException("Iterator Error: Invalid Iterator");
            if (it==dex->byId.begin()) throw IteratorException("Iterator Error: Out Of Bound");
            --it; return *this;
        }
        iterator operator++(int) { auto tmp=*this; ++(*this); return tmp; }
        iterator operator--(int) { auto tmp=*this; --(*this); return tmp; }
        iterator & operator = (const iterator &rhs) { dex=rhs.dex; it=rhs.it; return *this; }
        bool operator == (const iterator &rhs) const { return dex==rhs.dex && it==rhs.it; }
        bool operator != (const iterator &rhs) const { return !( *this == rhs ); }
        Pokemon & operator*() const { return const_cast<Pokemon&>(it->second); }
        Pokemon *operator->() const { return const_cast<Pokemon*>(&it->second); }
    };

    iterator begin() { return iterator(this, byId.begin()); }
    iterator end() { return iterator(this, byId.end()); }
};
