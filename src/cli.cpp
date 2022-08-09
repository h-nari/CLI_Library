#include <stdarg.h>
#include <ctype.h>
#include "cli.h"

int verbose = 0;

bool get_int(const char *str, int *pVal, const char **pNext) {
  const char *p = str;
  int sgn = 1;
  int val = 0;

  while (isspace(*p)) p++;
  if (*p == '-') {
    sgn = -1;
    p++;
  }
  if (isdigit(*p)) {
    while (isdigit(*p)) {
      val = val * 10 + *p - '0';
      p++;
    }
    if (pVal) *pVal = val * sgn;
    if (pNext) *pNext = p;
    return true;
  } else {
    if (pNext) *pNext = p;
    return false;
  }
};

bool get_word(const char *str, char *buf, size_t bufLen, const char **pNext) {
  const char *p = str;
  size_t i = 0;
  while (isspace(*p)) p++;
  if (*p) {
    while (*p && !isspace(*p) && i < bufLen - 1) buf[i++] = *p++;
    buf[i] = 0;

    if (pNext) *pNext = p;
    return true;
  } else {
    if (pNext) *pNext = p;
    return false;
  }
}

static bool help_cmd(CLI *cli, const char *args) {
  cli->printf("Command List\n");
  _Cmd *p = cli->m_cmd_list;
  if (p)
    for (; p; p = (_Cmd *)p->_next)
      cli->printf("  %-8s     %s\n", p->_name, p->_explain ? p->_explain : "");
  else
    cli->printf(" no command registered\n");
  return cli->printf("\n");
}

static bool set_cmd(CLI *cli, const char *args) {
  const char *p = args;
  int val;
  char name[16];

  if (!get_word(p, name, sizeof name, &p)) {  // no argument: set
    cli->printf("Param Int list\n");
    _Param_int *param = cli->m_param_int_list;
    if (param)
      for (; param; param = (_Param_int *)param->_next)
        cli->printf("  %-8s = %6d    # %s\n", param->_name, *param->_pVal,
                    param->_explain ? param->_explain : "");
    else
      cli->printf("  no parameter registered\n");
    cli->printf("\n");
  } else {
    _Param_int *param;
    if (!cli->param_int_find(name, &param))
      return cli->error("%s not found", name);
    else {
      if (!get_int(p, &val, NULL))  // no value: set name
        cli->printf("%d\n", *(param->_pVal));
      else {  // set name value
        *(param->_pVal) = val;
      }
    }
  }
  return true;
}

CLI::CLI() {
  m_serial = NULL;
  m_cmd_list = NULL;
  m_param_int_list = NULL;
  m_ri = 0;
  m_bEcho = true;
  cmd("help", "show cmd list", help_cmd);
  cmd("h", "show cmd list", help_cmd);
  cmd("set", "show/set parameters", set_cmd);
  param_int("verbose", "verbose setting", &verbose);
}

void CLI::cmd(const char *name, const char *explain, cmd_func_t func) {
  _Cmd *c = new _Cmd(name, explain, func);
  _NameList::insert((_NameList **)&m_cmd_list, c);
}

void CLI::param_int(const char *name, const char *explain, int *pVal) {
  _Param_int *n = new _Param_int(name, explain, pVal);
  _NameList::insert((_NameList **)&m_param_int_list, n);
}

bool CLI::printf(const char *fmt, ...) {
  if (m_serial) {
    char buf[80];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    m_serial->print(buf);
    va_end(ap);
  }
  return true;
}

bool CLI::error(const char *fmt, ...) {
  if (m_serial) {
    char buf[80];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    m_serial->print(buf);
    va_end(ap);
    m_serial->println();
  }
  return false;
}

void CLI::init(Stream *serial) {
  this->m_serial = serial;
  if (this->m_serial) this->printf("\nReady\n$ ");
}

void CLI::update(void) {
  if (m_serial && m_serial->available()) {
    if (m_ri < sizeof(m_rxbuf) - 1) {
      int c = m_serial->read();
      if (c == '\b') {
        if (m_ri > 0) {
          m_ri--;
          if (m_bEcho) this->printf("\b \b");
        }
      } else if (c == '\n' || c == 0) {
        /* do nothing */
      } else if (c != '\r') {
        m_rxbuf[m_ri++] = c;
        if (m_bEcho) this->printf("%c", c);
      } else {
        char name[16];
        const char *arg;
        m_rxbuf[m_ri] = 0;
        if (m_bEcho) this->printf("\n");
        if (get_word(m_rxbuf, name, sizeof name, &arg)) {
          _Cmd *cmd;
          if (cmd_find(name, &cmd)) {
            m_cmd = name;
            cmd->_func(this, arg);
          } else {
            this->printf("command \"%s\" not defined\n", name);
          }
        }
        this->printf("$ ");
        m_ri = 0;
      }
    }
  }
}
