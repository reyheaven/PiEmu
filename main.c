/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#include "common.h"

/**
 * Prints the command line options
 */
static void
cmdline_usage(int argc, char **argv)
{
  printf("%s [options] image\n", argc > 0 ? argv[0] : "emulate");
  printf("Options:\n");
  printf("  --quiet         Does not dump CPU state\n");
  printf("  --graphics      Emulate framebuffer\n");
  printf("  --memory=size   Specify memory size in bytes\n");
  printf("  --addr=addr     Specify kernel start address\n");
  printf("  --help          Print this message\n");
}

/**
 * Parses command line arguments using getopt
 * @param emu  Reference to the emulator
 * @param argc Number of command line arguments
 * @param argv Argument values
 * @return Nonzero if arguments are valid
 */
static int
cmdline_parse(emulator_t *emu, int argc, char **argv)
{
  struct option options[] =
  {
    { "graphics",  no_argument,        &emu->graphics,     1 },
    { "help",      no_argument,        &emu->usage,        1 },
    { "quiet",     no_argument,        &emu->quiet,        1 },
    { "nes",       no_argument,        &emu->nes_enabled,  1 },
    { "memory",    required_argument, 0,                 'm' },
    { "addr",      required_argument, 0,                 'a' },
    { "gpio-test", required_argument, 0,                 'i' },
    { 0, 0, 0, 0 }
  };

  int c, index;

  /* Default args */
  emu->mem_size = 0x10000;

  /* Read all arguments */
  while ((c = getopt_long(argc, argv, "vghsm:a:", options, &index)) != -1)
  {
    switch (c)
    {
      case 'q': emu->quiet = 1; break;
      case 'g': emu->graphics = 1; break;
      case 'h': emu->usage = 1; break;
      case 'm':
      {
        /* Handle prefixes */
        int length = strlen(optarg);
        switch (optarg[length - 1])
        {
          case 'm': case 'M':
          {
            optarg[length - 1] = '\0';
            sscanf(optarg, "%zu", &emu->mem_size);
            emu->mem_size *= 1024 * 1024;
            break;
          }
          case 'k': case 'K':
          {
            optarg[length - 1] = '\0';
            sscanf(optarg, "%zu", &emu->mem_size);
            emu->mem_size *= 1024;
            break;
          }
          default:
          {
            sscanf(optarg, "%zu", &emu->mem_size);
          }
        }
        break;
      }
      case 'a':
      {
        sscanf(optarg, "%u", &emu->start_addr);
        break;
      }
      case 'i':
      {
        sscanf(optarg, "%u", &emu->gpio_test_offset);
        break;
      }
      case 0:
      {
        /* Flag set */
        break;
      }
      case '?':
      {
        /* Error */
        return 0;
      }
    }
  }

  /* Read image source */
  emu->image = optind >= argc ? NULL : argv[optind];
  return 1;
}

/**
 * Checks the validity of the arguments
 * @param argc Number of command line arguments
 * @param argv Argument values
 * @return Nonzero if arguments are valid
 */
static int
cmdline_check(emulator_t *emu, int argc, char **argv)
{
  if (emu->usage)
  {
    cmdline_usage(argc, argv);
    return 0;
  }

  /* Image source */
  if (!emu->image)
  {
    fprintf(stderr, "No kernel image specified.\n");
    return 0;
  }

  /* Memory size at least 64kb */
  if (emu->mem_size < 0x10000)
  {
    fprintf(stderr, "Must specify a minimum of 64kb of memory.\n");
    return 0;
  }

  return 1;
}

/**
 * Entry point of the application
 * @param argc Number of command line arguments
 * @param argv Argument values
 * @return EXIT_SUCCESS if everything goes fine
 */
int
main(int argc, char **argv)
{
  emulator_t emu;
  memset(&emu, 0, sizeof(emulator_t));

  /* In case of an error, code will jump here */
  if (setjmp(emu.err_jmp))
  {
    if (emu.err_msg)
    {
      fprintf(stderr, "ERROR: %s\n", emu.err_msg);
    }

    emulator_destroy(&emu);
    return EXIT_FAILURE;
  }

  /* Parse command line arguments */
  if (!cmdline_parse(&emu, argc, argv) || !cmdline_check(&emu, argc, argv))
  {
    return EXIT_FAILURE;
  }

  /* Run the emulator */
  emulator_init(&emu);
  emulator_load(&emu, emu.image);

  while (emulator_is_running(&emu))
  {
    emulator_tick(&emu);
  }

  if (!emu.quiet)
  {
    emulator_dump(&emu);
  }

  emulator_destroy(&emu);

  return EXIT_SUCCESS;
}
