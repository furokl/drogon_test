#include <drogon/drogon.h>
#include <chrono>

using namespace drogon;
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

void p_login(const HttpRequestPtr &req, Callback &&callback) {
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    std::string user = req->getParameter("user");
    std::string passwd = req->getParameter("passwd");

    if (user == "user" && 
        // TO DO Use Argon2 or BCrypt
        utils::getMd5("jadsjhdsajkh" + passwd) == "5B5299CF4CEAE2D523315694B82573C9")
    {
        req->session()->insert("loggedIn", true);
        resp->setBody("<script>window.location.href = \"/\";</script>");
        callback(resp);
    }
    else
    {
        resp->setStatusCode(k401Unauthorized);
        resp->setBody("<script>window.location.href = \"/\";</script>");
        callback(resp);
    }
}

int main()
{

    LOG_INFO << "Server started...";
    app()
        .enableSession(24h)
        //.loadConfigFile("../config.json")
        .addListener("192.168.0.105", 80)
        .registerHandler("/", &p_default, { Get })
        .registerHandler("/logout", &p_logout, { Post })
        .registerHandler("/login", &p_login, { Post })
        .run();
}