#ifndef _cli_h_
#define _cli_h_

#include <Arduino.h>

class CLI;
typedef bool (*cmd_func_t)(class CLI *cli, const char *arg);

class _NameList;
class _NameList {
 public:
  const char *_name;
  _NameList *_next;

  _NameList(const char *name) {
    _name = name;
    _next = NULL;
  }

  static bool find(_NameList *list, const char *name, _NameList **pPtr) {
    for (_NameList *p = list; p; p = p->_next)
      if (strcmp(name, p->_name) == 0) {
        if (pPtr) *pPtr = p;
        return true;
      }
    return false;
  }

  static void insert(_NameList **pList, _NameList *node) {
    _NameList *p = *pList;
    _NameList *p0 = NULL;
    while (p && strcmp(p->_name, node->_name) < 0) {
      p0 = p;
      p = p->_next;
    }
    if (p0)
      p0->_next = node;
    else
      *pList = node;
    node->_next = p;
  }
};

class _Cmd : public _NameList {
 public:
  cmd_func_t _func;
  const char *_explain;

  _Cmd(const char *name, const char *explain, cmd_func_t func)
      : _NameList(name) {
    _func = func;
    _explain = explain;
  }
};

class _Param_int : public _NameList {
 public:
  int *_pVal;
  const char *_explain;

  _Param_int(const char *name, const char *explain, int *pVal)
      : _NameList(name) {
    _pVal = pVal;
    _explain = explain;
  }
};

class CLI {
 public:
  Stream *m_serial;
  _Cmd *m_cmd_list;
  _Param_int *m_param_int_list;
  int m_ri;
  char m_rxbuf[80];
  const char *m_cmd;

 public:
  CLI();
  void init(Stream *serial);
  void update(void);
  void cmd(const char *name, const char *explain, cmd_func_t func);
  void param_int(const char *name, const char *explain, int *pVal);
  bool printf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
  bool error(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
  bool param_int_find(const char *name, _Param_int **pParam) {
    return _NameList::find(m_param_int_list, name, (_NameList **)pParam);
  }
  bool cmd_find(const char *name, _Cmd **pCmd) {
    return _NameList::find(m_cmd_list, name, (_NameList **)pCmd);
  }
};

bool get_int(const char *str, int *pVal, const char **pNext);
bool get_word(const char *str, char *buf, size_t bufLen, const char **pNext);

#endif /* _cli_h_ */
