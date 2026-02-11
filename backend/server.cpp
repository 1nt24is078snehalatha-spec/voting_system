#define CROW_USE_BOOST
#include <boost/asio.hpp>
#include "crow/crow.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

using namespace std;

/* ================= CONFIG ================= */
const string ADMIN_TOKEN = "TECHCLUB_ADMIN_2026";
const string DATA_FILE = "data.json";

/* ================= STRUCTS ================= */
struct VoteInfo
{
    string userHash;
    string email;
    string usn;
    string year;
    string option;
};

struct Question
{
    int id;
    string text;
    vector<string> options;
    vector<VoteInfo> votes;
    unordered_map<string, int> voteCount;
};

/* ================= STORAGE ================= */
unordered_map<int, Question> questions;
int nextQuestionId = 1;

/* ================= UTILS ================= */
string toLower(string s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool isValidUser(const string &email, const string &usn)
{
    if (email.size() < 10 || usn.size() < 10)
        return false;
    return toLower(email.substr(0, 10)) == toLower(usn.substr(0, 10));
}

string simpleHash(const string &s)
{
    unsigned long h = 5381;
    for (char c : s)
        h = ((h << 5) + h) + (unsigned char)tolower(c);
    return to_string(h);
}

bool alreadyVoted(Question &q, const string &hash)
{
    for (auto &v : q.votes)
        if (v.userHash == hash)
            return true;
    return false;
}

/* ================= PERSISTENCE ================= */
void saveToFile()
{
    crow::json::wvalue root;
    for (auto &p : questions)
    {
        auto &q = p.second;
        string id_key = to_string(q.id);
        root[id_key]["id"] = q.id;
        root[id_key]["text"] = q.text;
        root[id_key]["options"] = q.options;

        int v_idx = 0;
        for (auto &v : q.votes)
        {
            root[id_key]["votes"][v_idx]["hash"] = v.userHash;
            root[id_key]["votes"][v_idx]["email"] = v.email;
            root[id_key]["votes"][v_idx]["usn"] = v.usn;
            root[id_key]["votes"][v_idx]["year"] = v.year;
            root[id_key]["votes"][v_idx]["option"] = v.option;
            v_idx++;
        }
    }
    ofstream out(DATA_FILE);
    out << root.dump();
}

void loadFromFile()
{
    ifstream in(DATA_FILE);
    if (!in.is_open())
        return;
    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    if (content.empty())
        return;

    auto data = crow::json::load(content);
    if (!data)
        return;

    questions.clear();
    nextQuestionId = 1;

    for (auto &item : data)
    {
        Question q;
        q.id = (int)item["id"].i();
        q.text = item["text"].s();

        if (item.has("options"))
        {
            for (auto &o : item["options"])
            {
                q.options.push_back(o.s());
                q.voteCount[o.s()] = 0; // Initialize counts
            }
        }

        if (item.has("votes"))
        {
            for (auto &v : item["votes"])
            {
                if (v.has("hash") && v.has("email") && v.has("usn"))
                {
                    VoteInfo vi;
                    vi.userHash = v["hash"].s();
                    vi.email = v["email"].s();
                    vi.usn = v["usn"].s();
                    vi.year = v.has("year") ? string(v["year"].s()) : "";
                    vi.option = v.has("option") ? string(v["option"].s()) : "";
                    q.votes.push_back(vi);
                    q.voteCount[vi.option]++;
                }
            }
        }
        questions[q.id] = q;
        nextQuestionId = max(nextQuestionId, q.id + 1);
    }
}

/* ================= CORS MIDDLEWARE ================= */
struct CORS
{
    struct context
    {
    };
    void before_handle(crow::request &req, crow::response &res, context &ctx)
    {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Admin-Token, Authorization");
        res.add_header("Access-Control-Allow-Credentials", "true");

        // If it's an OPTIONS request, answer it immediately and don't look for a route
        if (req.method == "OPTIONS"_method)
        {
            res.code = 204; // No Content
            res.end();
        }
    }
    void after_handle(crow::request &req, crow::response &res, context &ctx) {}
};

/* ================= MAIN ================= */
int main()
{
    crow::SimpleApp app;
    loadFromFile();
    CROW_ROUTE(app, "/admin/question").methods("POST"_method, "OPTIONS"_method)([](const crow::request &req, crow::response &res)
                                                                                {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");

    if (req.method == "OPTIONS"_method) { res.code = 200; res.end(); return; }

    auto data = crow::json::load(req.body);
    if (!data) { res.code = 400; res.end(); return; }

    Question q;
    q.id = nextQuestionId++; // Ensure this global variable is incrementing
    q.text = data["question"].s();
    
    for (auto& o : data["options"]) {
        string optText = o.s();
        q.options.push_back(optText);
        q.voteCount[optText] = 0; // Initialize counts to zero
    }

    questions[q.id] = q;
    saveToFile(); // Important to persist the new question

    res.code = 200;
    res.write("Success");
    res.end(); });
    // GET QUESTION
    CROW_ROUTE(app, "/latest")([](const crow::request &req, crow::response &res)
                               {
    res.add_header("Access-Control-Allow-Origin", "*");
    if(questions.empty()) { res.code = 404; res.end(); return; }
    
    int maxId = -1;
    for (auto const& [id, q] : questions) { if (id > maxId) maxId = id; }
    auto &q = questions[maxId];

    crow::json::wvalue j;
    j["id"] = q.id;
    j["question"] = q.text;
    j["options"] = q.options;
    
    // ADD THIS LINE so the frontend can see the results
    for(auto const& opt : q.options) {
        j["results"][opt] = q.voteCount[opt];
    }

    res.body = j.dump();
    res.end(); });

    // VOTE
    CROW_ROUTE(app, "/vote").methods("POST"_method, "OPTIONS"_method)([](const crow::request &req, crow::response &res)
                                                                      {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");

    if (req.method == "OPTIONS"_method) { res.code = 204; res.end(); return; }

    auto data = crow::json::load(req.body);
    int qId = data["id"].i();
    string inputEmail = data["email"].s();
    string inputUsn = data["usn"].s();
    
    // 1. MATCH CHECK (First 10 Characters)
    // Example: 1MS22CS001@gmail.com and 1MS22CS001
    if (inputEmail.length() < 10 || inputUsn.length() < 10 || 
        inputEmail.substr(0, 10) != inputUsn.substr(0, 10)) {
        res.code = 401; // Unauthorized
        res.write("Validation Failed: Email prefix and USN do not match.");
        res.end();
        return;
    }

    auto &q = questions[qId];
    
    // 2. DUPLICATE CHECK
    for(auto &v : q.votes) {
        if(v.email == inputEmail || v.usn == inputUsn) { 
            res.code = 409; 
            res.write("Duplicate Entry: This identity has already voted."); 
            res.end(); 
            return;
        }
    }

    // 3. SUCCESS: Save the vote
    q.votes.push_back({inputEmail + inputUsn, inputEmail, inputUsn, "2026", data["option"].s()});
    q.voteCount[data["option"].s()]++;
    saveToFile();
    
    res.code = 200;
    res.write("Success");
    res.end(); });

    app.port(18080).run();
}