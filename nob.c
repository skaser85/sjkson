#define NOB_IMPLEMENTATION
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"

Cmd cmd = {0};

static void usage(void)
{
    fprintf(stderr, "Usage: %s [<FLAGS>] [--] [<program args>]\n", flag_program_name());
    fprintf(stderr, "FLAGS:\n");
    flag_print_options(stderr);
}

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    char* prog = "";
    bool run = false;
    bool help = false;
    flag_str_var(&prog, "prog", "main", "Supply the file name to run (default is 'main')");
    flag_bool_var(&run, "run", false, "Run the program after compilation.");
    flag_bool_var(&help, "help", false, "Print this help message.");

    if (!flag_parse(argc, argv)) {
        usage();
        flag_print_error(stderr);
        return 1;
    }

    if (help) {
        usage();
        return 0;
    }
    
    char output_file_name[100];
    sprintf(output_file_name, "./%s", prog);

    char input_file_name[100];
    sprintf(input_file_name, "%s.c", prog);

    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-o", output_file_name, input_file_name);
    /*
    cmd_append(&cmd, "-I./raylib-5.5_linux_amd64/include/");
    cmd_append(&cmd, "-L./raylib-5.5_linux_amd64/lib/");
    cmd_append(&cmd, "-l:libraylib.a");
    cmd_append(&cmd, "-lm");
    */
    if (!cmd_run(&cmd)) return 1;

    if (run) {
        cmd_append(&cmd, output_file_name);
        da_append_many(&cmd, argv, argc);
        if (!cmd_run(&cmd)) return 1;
    }

    return 0;
}
