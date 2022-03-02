//-------------------------------BAZA DE DATE -----------------------------
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <sqlite3.h>

sqlite3 *db1;
sqlite3 *db2;
class BazaDate
{
private:
  const char *data = "Functia callback a fost apelata";
  char sql[1024];
  sqlite3_stmt *res;
  const char *tail;
  int rec_count = 0;
  int rc;

public:
  BazaDate() {}

  void begin()
  {
    sqlite3_initialize();
    // list SPIFFS contents
    File root = SPIFFS.open("/");
    if (!root)
    {
      Serial.println("- failed to open directory");
      return;
    }
    if (!root.isDirectory())
    {
      Serial.println(" - not a directory");
      return;
    }
    File file = root.openNextFile();
    boolean agenda = false;
    boolean setari = false;

    while (file)
    {
      if (!file.isDirectory())
      {
        if (String(file.name()) == "agenda.db")
        {
          agenda = true;
        }
        else if (String(file.name()) == "setari.db")
        {
          {
            setari = true;
          }
        }
        file = root.openNextFile();
      }
    }
    if (!agenda && !setari)
    {
      db_open("/spiffs/agenda.db", &db1);
      db_exec(db1, "CREATE TABLE IF NOT EXISTS tablenrtel (nrtel);");
      db_exec(db1, "DELETE FROM tablenrtel WHERE nrtel IS NULL OR trim(nrtel) = '' ");
      sqlite3_close(db1);
      db_open("/spiffs/setari.db", &db2);
      db_exec(db2, "CREATE TABLE IF NOT EXISTS setari (id INTEGER, jsonDb);");
      sqlite3_close(db2);
    }
  }
  void settings()
  {
    String buff;
    if (db_open("/spiffs/setari.db", &db2))
    {
      return;
    }

    String sql = "SELECT * FROM setari";
    rc = sqlite3_prepare_v2(db2, sql.c_str(), strlen(sql.c_str()), &res, &tail);
    if (rc == SQLITE_OK && sqlite3_step(res) == SQLITE_ROW)
    {
        buff = (const char *)sqlite3_column_text(res, 1);
    }

    if (buff.length() == 0)
    {
      db_exec(db2, "DELETE FROM setari WHERE jsonDb=''");
      db_exec(db2, "INSERT INTO setari VALUES (0, '');");
    }

    sqlite3_finalize(res);
    sqlite3_close(db2);
 
    if (jsonToSettings(buff))
    {
      Serial.println("|DB|SETARI| --> Sau sincronizat setarile cu cele din DB ");
    }
    else
    {
      Serial.println("|ERR!|DB|SETARI| --> Setarile nu au fost sincronizate cu DB! ");
    }
  }


  void settingsToDB(String settings)
  {
    if (db_open("/spiffs/setari.db", &db2))
    {
      return;
    }

    String sql = "UPDATE  setari SET jsonDb = (?) WHERE id = 0;";
    rc = sqlite3_prepare_v2(db2, sql.c_str(), strlen(sql.c_str()), &res, &tail);
    if (rc != SQLITE_OK)
    {
      Serial.println("|ERR!|DB|SETARI| --> Nu s-au putut citi setarile !");
      sqlite3_close(db2);
      return;
    }
    else
    {
      sqlite3_bind_text(res, 1, settings.c_str(), strlen(settings.c_str()), SQLITE_STATIC);
      if (sqlite3_step(res) != SQLITE_DONE)
      {
        Serial.println("|DB|SETARI| --> Sau actualizat setarile din DB!");
        sqlite3_finalize(res);
        sqlite3_close(db2);
        return;
      }
    }
    sqlite3_finalize(res);
    sqlite3_close(db2);
  }

  // cauta nr de tel in baza de date
  boolean findNrTel(String nrTel)
  {
    Serial.println("|DB|AGENDA| --> Se cauta nr de tel in agenda... !");
    if (db_open("/spiffs/agenda.db", &db1))
    {
      return false;
    }

    String sql = "SELECT * FROM tablenrtel WHERE nrtel LIKE '";
    sql += nrTel;
    sql += "'";

    rc = sqlite3_prepare_v2(db1, sql.c_str(), strlen(sql.c_str()), &res, &tail);
    while (rc == SQLITE_OK && sqlite3_step(res) == SQLITE_ROW)
    {
      rec_count = sqlite3_column_int(res, 0);
      if (rec_count > 0)
      {
        Serial.println("|DB|AGENDA| --> Numarul de tel: " + nrTel + " a fost gasit in agenda.");
        sqlite3_finalize(res);
        sqlite3_close(db1);
        return true;
      }
    }
    sqlite3_finalize(res);
    sqlite3_close(db1);
    Serial.println("|DB|AGENDA| --> Numarul de tel: " + nrTel + " nu exista in agenda.");
    return false;
  }

  // sterge numere de telefon cu dubluri daca exista
  boolean delNrTel(String nrTel)
  {
    if (db_open("/spiffs/agenda.db", &db1))
    {
      return false;
    }
    String sql = "DELETE FROM tablenrtel WHERE nrtel LIKE '";
    sql += nrTel;
    sql += "'";
    rc = sqlite3_prepare_v2(db1, sql.c_str(), 1000, &res, &tail);
    while (rc == SQLITE_OK && sqlite3_step(res) == SQLITE_ROW)
    {
      rec_count = sqlite3_column_int(res, 0);
      if (rec_count > 0)
      {
        sqlite3_finalize(res);
        sqlite3_close(db1);
        Serial.println("|DB|AGENDA| --> Numarul de tel: " + nrTel + " a fost sters.");
        return true;
      }
    }

    sqlite3_finalize(res);
    sqlite3_close(db1);
    Serial.println("|DB|AGENDA| --> Numarul de tel: " + nrTel + " nu sa sters.");
    return false;
  }
  // adauga numere de telefon in baza de date
  void insertNrTel(String nrTel)
  {
    if (!findNrTel(nrTel))
    {
      if (db_open("/spiffs/agenda.db", &db1))
      {
        return;
      }

      String sql = "INSERT INTO tablenrtel VALUES (?);";
      rc = sqlite3_prepare_v2(db1, sql.c_str(), strlen(sql.c_str()), &res, &tail);
      if (rc != SQLITE_OK)
      {
        Serial.printf("ERROR preparing sql: %s\n", sqlite3_errmsg(db1));
        sqlite3_close(db1);
      }
      else
      {
        sqlite3_bind_text(res, 1, nrTel.c_str(), strlen(nrTel.c_str()), SQLITE_STATIC);
        if (sqlite3_step(res) != SQLITE_DONE)
        {
          Serial.println("|DB|AGENDA| --> Numarul de tel: " + nrTel + " a sa putut adauga in agenda.");
          sqlite3_finalize(res);
          sqlite3_close(db1);
          return;
        }
      }
      sqlite3_finalize(res);
      sqlite3_close(db1);
      Serial.println("|DB|AGENDA| --> Numarul de tel: " + nrTel + " sa adaugat in agenda.");
    }
  }

  // Initializare baza de date
  static int callback(void *data, int argc, char **argv, char **azColName)
  {

    for (int i = 0; i < argc; i++)
    {
      Serial.println(argv[i]);
    }
    return 0;
  }

  int db_open(const char *filename, sqlite3 **db)
  {
    int rc = sqlite3_open(filename, db);
    if (rc)
    {
      Serial.println("|ERR!|DB| --> Nu sa putut citi fisierul: " + String(filename));
      return rc;
    }
    else
    {
      Serial.println("|DB| --> Fisierul: " + String(filename) + " a fost citit cu succes");
    }
    return rc;
  }

  char *zErrMsg = 0;
  int db_exec(sqlite3 *db, const char *sql)
  {
    int rc = sqlite3_exec(db, sql, callback, (void *)data, &zErrMsg);
    if (rc != SQLITE_OK)
    {
      Serial.println("|ERR!|DB|" + String(zErrMsg) + "|  --> Interogare sql gresita!");
      sqlite3_free(zErrMsg);
    }
    return rc;
  }
};
BazaDate DB;
