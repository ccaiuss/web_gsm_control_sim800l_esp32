#include <HTTPServer.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <WebsocketHandler.hpp>
//webserver
using namespace httpsserver;
HTTPServer *webServer;
// Max clients to be connected to the chat
#define MAX_CLIENTS 10
#define HEADER_USERNAME "admin"
#define HEADER_GROUP "abctools"

const char *http_username = "admin";
const char *http_password = "admin";
char contentTypes[][2][32] = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {"", ""},};
void handleSPIFFS(HTTPRequest *req, HTTPResponse *res)
{
  if (req->getMethod() == "GET" && req->getHeader(HEADER_GROUP) == "administrator")
  {
    if (req->getRequestString() != "/web/favicon.ico")
    {
      res->setHeader("Cache-Control", "max-age=2592000");
    }

    std::string reqFile = req->getRequestString() == "/" ? "/index.html" : req->getRequestString();

    std::string filename = "/web" + reqFile;

    if (!SPIFFS.exists(filename.c_str()))
    {
      res->setStatusCode(404);
      res->setStatusText("Not found");
      res->println("404 Not Found");
      return;
    }

    File file = SPIFFS.open(filename.c_str());

    res->setHeader("Content-Length", httpsserver::intToString(file.size()));

    int cTypeIdx = 0;
    do
    {
      if (reqFile.rfind(contentTypes[cTypeIdx][0]) != std::string::npos)
      {
        res->setHeader("Content-Type", contentTypes[cTypeIdx][1]);
        break;
      }
      cTypeIdx += 1;
    } while (strlen(contentTypes[cTypeIdx][0]) > 0);

    uint8_t buffer[256];
    size_t length = 0;
    do
    {
      length = file.read(buffer, 256);
      res->write(buffer, length);
    } while (length > 0);

    file.close();
  }
  else
  {
    req->discardRequestBody();
    res->setStatusCode(405);
    res->setStatusText("Method not allowed");
    res->println("405 Method not allowed");
  }
}
 
void middlewareAuthentication(HTTPRequest *req, HTTPResponse *res, std::function<void()> next)
{

  req->setHeader(HEADER_USERNAME, "");
  req->setHeader(HEADER_GROUP, "");

  std::string reqUsername = req->getBasicAuthUser();
  std::string reqPassword = req->getBasicAuthPassword();

  if (reqUsername.length() > 0 && reqPassword.length() > 0)
  {
    bool authValid = true;
    std::string group = "";
    if (reqUsername == "admin" && reqPassword == "admin")
    {
      group = "administrator";
    }
    else
    {
      authValid = false;
    }
    if (authValid)
    {
      req->setHeader(HEADER_USERNAME, reqUsername);
      req->setHeader(HEADER_GROUP, group);
      next();
    }
    else
    {
      res->setStatusCode(401);
      res->setStatusText("Unauthorized");
      res->setHeader("Content-Type", "text/plain");
      res->setHeader("WWW-Authenticate", "Basic realm=\"ESP32 privileged area\"");
      res->println("401. Unauthorized");
    }
  }
  else
  {
    next();
  }
}

void middlewareAuthorization(HTTPRequest *req, HTTPResponse *res, std::function<void()> next)
{
  std::string username = req->getHeader(HEADER_USERNAME);

  if (username == "" && req->getRequestString().substr(0, 1) == "/")
  {
    res->setStatusCode(401);
    res->setStatusText("Unauthorized");
    res->setHeader("Content-Type", "text/plain");
    res->setHeader("WWW-Authenticate", "Basic realm=\"ESP32 privileged area\"");
    res->println("401. Unauthorized ");
  }
  else
  {
    next();
  }
}
