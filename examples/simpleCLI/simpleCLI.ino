#include <Arduino.h>
#include <cli.h>

CLI cli;

bool cmd_sum(CLI *cli, const char *arg) {
  const char *p = arg;
  int sum = 0, iVal;

  while (get_int(p, &iVal, &p)) sum += iVal;
  return cli->printf("sum = %d\n", sum);
}

bool cmd_factorize(CLI *cli, const char *arg) {
  int n, i, c = 0;
  if (!get_int(arg, &n, NULL)) return cli->error("integer argment required");
  if (n < 2) return cli->error("bad interger %d, must be >1", n);
  cli->printf("%d =", n);
  for (i = 2; i <= n; i++) {
    while (n % i == 0) {
      cli->printf(" %s%d", c++ ? "* ": "", i);
      n /= i;
    }
  }
  return cli->printf("\n");
}

void setup(void) {
  Serial.begin(115200);
  cli.init(&Serial);
  cli.cmd("sum", "Calculate the sum of the arguments", cmd_sum);
  cli.cmd("f", "factorize the argument", cmd_factorize);
}

void loop(void) { cli.update(); }