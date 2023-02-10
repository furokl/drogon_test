#include <drogon/drogon.h>
#include <chrono>
#define USE_POSTGRESQL
using namespace drogon;
using namespace drogon::orm;
using namespace std::chrono_literals;

typedef std::function<void(const HttpResponsePtr &)> Callback;

void p_default(const HttpRequestPtr &req, Callback &&callback) {
    bool loggedIn =
        req->session()->getOptional<bool>("loggedIn").value_or(false);
    HttpResponsePtr resp =
        (!loggedIn) ?
        HttpResponse::newHttpViewResponse("LoginPage") :
        HttpResponse::newHttpViewResponse("LogoutPage");
    callback(resp);
}

void p_logout(const HttpRequestPtr &req, Callback &&callback) {
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    req->session()->erase("loggedIn");
    resp->setBody("<script>window.location.href = \"/\";</script>");
    callback(resp);
}

auto p_login(const HttpRequestPtr req) -> Task<HttpResponsePtr> {
    auto esq = app().getDbClient("furokl_db");
    try
    {
        std::string user = req->getParameter("user");
        std::string passwd = req->getParameter("passwd");
        auto result = co_await esq->execSqlCoro( // execSqlSync
            "SELECT COUNT(*) FROM \"Account\" "
            "WHERE "
            "\"username\" = $1 AND "
            "\"password\" = $2",
            user, passwd); 
        auto resp = HttpResponse::newHttpResponse();
        if (result[0][0].as<size_t>() > 0)
        {
            req->session()->insert("loggedIn", true);
            resp->setBody("<script>window.location.href = \"/\";</script>");
            co_return resp;
        }
        else 
        {
            resp->setStatusCode(k401Unauthorized);
            resp->setBody("<script>window.location.href = \"/\";</script>");
            co_return resp;
        }
    }
    catch (const DrogonDbException &e)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody(e.base().what());
        co_return resp;
    }
}

int main()
{
    app()
        .loadConfigFile("../config.json")
        .registerHandler("/", &p_default, { Get })
        .registerHandler("/logout", &p_logout, { Post })
        .registerHandler("/login", &p_login, { Post })
        .run();
}