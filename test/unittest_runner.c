#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <inttypes.h>
#include "cmockery.h"

#include "t24.h"

void test_Bootstrap(void **state) {
  assert_int_equal(0LL, 0LL);
}


void modalfatalbox(char *fmt, ...) {}

void t24_basic_highlight(termchar *newline, int cols);

termline *newline(const char* chars) {
  int cols = strlen(chars);
  termline *line;
  int j;

  line = snew(termline);
  line->chars = snewn(cols, termchar);
  for (j = 0; j < cols; j++) {
    line->chars[j].chr = chars[j];
    line->chars[j].attr = '.';
  }

  return line;
}

void freeline(termline *line) {
  if (line) {
    sfree(line->chars);
    sfree(line);
  }
}

const char* decode_colors(const termchar* chars, int cols) {
  int current_color = -1;
  int letter = 'a' - 1;
  static char buf[1024];
  int i;
  for (i = 0; i < cols; ++i) {
    int color = chars[i].attr;
    if (color == '.') {
      buf[i] = color;
      current_color = -1;
    } else {
      if (current_color != color) {
        current_color = color;
        letter += 1;
      }
      buf[i] = letter;
    }
  }
  buf[cols] = 0;
  return buf;
}

const char* syntax(const char *chars) {
  size_t sz = strlen(chars);
  termline* line = newline(chars);
  t24_basic_highlight(line->chars, sz);
  const char* result = decode_colors(line->chars, sz);
  freeline(line);
  return result;
}

#define string_eq(a, b) assert_string_equal(syntax(a), b)

void test_Comments(void **state) {
  string_eq("1234 !--------", ".....aaaaaaaaa");
  string_eq("1234   !--------", ".......aaaaaaaaa");
  string_eq("1234 *--------", ".....aaaaaaaaa");
  string_eq("1234   *--------", ".......aaaaaaaaa");
  string_eq("1234 VAR ;* comments", ".........aaaaaaaaaaa");
  string_eq("1234 VAR // comments", ".........aaaaaaaaaaa");
  string_eq("1234 A = '// string'  // slash/shash comment", 
            ".........aaaaaaaaaaa..bbbbbbbbbbbbbbbbbbbbbb");
  string_eq("0052      YAC.BAL *= -1",
            ".......................");
}

void test_Ticket_dd6a19efa5_DATE(void **state) {
  string_eq("0029  ENTRIES<1, AC.STE.VALUE.DATE>    = TODAY", 
            "..............a...............................");
  string_eq("0036  ENTRIES<1, AC.STE.BOOKING.DATE>  = TODAY", 
            "..............a...............................");
  string_eq("0036  ENTRIES<1, AC.STE.TOOKING DATE>  = TODAY", 
            "..............a.................bbbb..........");
  string_eq("0036 DATE = TODAY", 
            ".....aaaa........");
  string_eq("0036  DATE = TODAY", 
            "......aaaa........");
}

void test_Ticket_e8e02762a0_V_TIME(void **state) {
  string_eq("0017     V.TIME = ''", 
            "..................aa");
  string_eq("0034     V.DELTA = TIME() - TIME1", 
            "...................aaaa..........");
}

void test_Ticket_0bcfac1fb6_READNEXT_FROM(void **state) {
  string_eq("0167     READNEXT ID FROM 9 ELSE DONE = 1 ", 
            ".........aaaaaaaa....bbbb.c.dddd........e.");
}

void test_Ticket_3f3645bab6_UNTIL_DO_REPEAT(void **state) {
  string_eq("0204     UNTIL DONE DO ", 
            ".........aaaaa......bb.");
  string_eq("0230     REPEAT", 
            ".........aaaaaa");
  string_eq("0231     REPEAT ", 
            ".........aaaaaa.");
}

void test_Ticket_cce4f3f8dc1b52fc_F_IN_FILE(void **state) {
  string_eq("0015     OPENSEQ V.DIR.IN, V.FILE.IN TO F.IN.FILE ELSE", 
            ".........aaaaaaa.....................bb...........cccc");
}

void test_Ticket_0a8c43b9ed_AND_OR(void **state) {
  string_eq("0003 IF PAR.1 EQ 1 AND (PAR.2 GT 3 OR PAR.2 LE 0) THEN", 
            ".....aa.......bb.c.ddd........ee.f.gg.......hh.i..jjjj");
}

void test_Ticket_3bfb7be594de7fd6_ON(void **state) {
  string_eq("0053    WRITESEQ RECON.REC ON F.FILE ELSE", 
            "........aaaaaaaa...........bb........cccc");
}

void test_Ticket_2018f2fd4a6af18b_ON_ERROR(void **state) {
  string_eq("0186    WRITE R.RECON TO F.RECON, V.RECON.ID ON ERROR",
            "........aaaaa.........bb.....................cc.ddddd");
}

void test_Ticket_3359b57f89_INSERT(void **state) {
  string_eq("1234 $INSERT abc",
            ".....aaaaaaa....");
  string_eq("1234     $INSERT abc",
            ".........aaaaaaa....");
}

void test_Ticket_11ffe01a8e_Case_Sensitivity(void **state) {
  string_eq("0001 PROGRAm TRY.1",
            "..................");
  string_eq("0003 CRt '111'",
            ".........aaaaa");
}

void test_Generic(void **state) {
  string_eq("0001 PROGRAM TRY.1",
            ".....aaaaaaa......");
}

void test_Common(void **state) {
  string_eq("0001 COMMON /BLOO.COMMON, ONE , TWO/ B$OO.ISIN.LIST, B$OO.SM.LST",
            ".....aaaaaa..bbbbbbbbbbbbbbbbbbbbbb.............................");
  string_eq("0001 COMMON /BLOO.COMMON/ B$OO.ISIN.LIST, B$OO.SM.LIST",
            ".....aaaaaa..bbbbbbbbbbb..............................");
}

void test_Label(void **state) {
  string_eq("0017  LABEL:", 
            "......aaaaaa");
  string_eq("0018 LABEL: ", 
            ".....aaaaaa.");
  string_eq("0019    LABEL: ", 
            "........aaaaaa.");
  string_eq("0019    LA:EL: ", 
            "........aaa....");
  string_eq("0019    LABEL: a = b", 
            "........aaaaaa......");
  string_eq("0019    LABEL: a = b:c:b", 
            "........aaaaaa..........");
  string_eq("0020    LABEL: // LABEL: ", 
            "........aaaaaa.bbbbbbbbbb");
}

int main(int argc, char* argv[]) {
  UnitTest tests[] = {
    unit_test(test_Bootstrap),
    unit_test(test_Comments),
    unit_test(test_Label),
    unit_test(test_Common),
    unit_test(test_Generic),
    unit_test(test_Ticket_dd6a19efa5_DATE),
    unit_test(test_Ticket_e8e02762a0_V_TIME),
    unit_test(test_Ticket_0bcfac1fb6_READNEXT_FROM),
    unit_test(test_Ticket_3f3645bab6_UNTIL_DO_REPEAT),
    unit_test(test_Ticket_cce4f3f8dc1b52fc_F_IN_FILE),
    unit_test(test_Ticket_0a8c43b9ed_AND_OR),
    unit_test(test_Ticket_3bfb7be594de7fd6_ON),
    unit_test(test_Ticket_2018f2fd4a6af18b_ON_ERROR),
    unit_test(test_Ticket_3359b57f89_INSERT),
    unit_test(test_Ticket_11ffe01a8e_Case_Sensitivity),
  };
  return run_tests(tests);
}
