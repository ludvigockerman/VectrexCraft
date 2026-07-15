#include <vectrex/bios.h>

#define TERMINAL_LENGTH 10

static char *terminalContent[TERMINAL_LENGTH];
static int currentLine = 0;

static void terminal_scroll(void)
{
    for (int i = 0; i < TERMINAL_LENGTH - 1; i++)
    {
        terminalContent[i] = terminalContent[i + 1];
    }

    terminalContent[TERMINAL_LENGTH - 1] = 0;
    currentLine = TERMINAL_LENGTH - 1;
}

void terminal_print(char *msg)
{
    if (currentLine >= TERMINAL_LENGTH)
    {
        terminal_scroll();
    }

    terminalContent[currentLine] = msg;
    currentLine++;
}

void terminal_render(void)
{
    for (int i = 0; i < currentLine && i < TERMINAL_LENGTH; i++)
    {
        if (terminalContent[i])
        {
            reset_beam();
            set_text_size(-6, 40);
            print_str_c((int8_t)(0 - (i * 10)), (int8_t)-120, terminalContent[i]);
        }
    }
}