/*
 * (C) Copyright 2005- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities granted to it by
 * virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
 */

#include "grib_tools.h"

grib_option grib_options[] = {
    /*  {id, args, help}, on, command_line, value*/
    { "s:", 0, 0, 1, 1, 0 },
    /*     {"r",0,0,0,1,0}, */
    /*     {"d:",0,0,0,1,0},*/
    /*     {"n:","noise percentage","\n\t\tAdd noise to the data values. The noise added is the given percentage of the data value.\n",0,1,0},*/
    { "p:", 0, 0, 1, 1, 0 },
    { "P:", 0, 0, 0, 1, 0 },
    { "w:", "key[:{s|d|i}]=value,key[:{s|d|i}]=value,...",
      "\n\t\tWhere clause.\n\t\tSet is only executed for BUFR messages matching all the "
      "key/value constraints.\n\t\tIf a BUFR message does not match the constraints it is"
      " copied unchanged\n\t\tto the output_file. This behaviour can be changed "
      "setting the option -S.\n\t\tFor each key a string (key:s), a double (key:d) or"
      " an integer (key:i)\n\t\ttype can be defined. Default type is string."
      "\n\t\tNote: only one -w clause is allowed.\n",
      0, 1, 0 },
    { "q", 0, 0, 1, 0, 0 },
    { "7", 0, 0, 0, 1, 0 },
    { "S", 0, 0, 0, 1, 0 },
    { "U", 0, 0, 1, 0, 0 },
    { "V", 0, 0, 0, 1, 0 },
    { "g", 0, 0, 0, 1, 0 },
    /*      {"G",0,0,0,1,0}, */
    { "T:", 0, 0, 1, 0, "B" },
    { "f", 0, 0, 0, 1, 0 },
    { "v", 0, 0, 0, 1, 0 },
    { "h", 0, 0, 0, 1, 0 },
};

const char* tool_description =
    "Sets key/value pairs in the input BUFR file and writes"
    "\n\teach message to the output_file."
    "\n\tIt fails when an error occurs (e.g. key not found).";
const char* tool_name       = "bufr_set";
const char* tool_online_doc = "https://confluence.ecmwf.int/display/ECC/bufr_set";
const char* tool_usage      = "[options] file file ... output_file";

int grib_options_count = sizeof(grib_options) / sizeof(grib_option);

int main(int argc, char* argv[])
{
    return grib_tool(argc, argv);
}

int grib_tool_before_getopt(grib_runtime_options* options)
{
    return 0;
}

int grib_tool_init(grib_runtime_options* options)
{
    if (options->set_values_count == 0 && !options->repack && !options->constant) {
        printf("ERROR: please provide some keys to set through the -s option or use the -r/-d options\n");
        exit(1);
    }
    if (options->verbose)
        options->print_header = 1;
    /*if (grib_options_on("n:")) {
    noise=atof(grib_options_get_option("n:"));
    options->repack=1;
  }*/

    if (grib_options_on("n:") && grib_options_on("d:")) {
        printf("Error: -n and -d options are incompatible. Choose one of the two please.\n");
        exit(1);
    }

    // if (options->outfile && options->outfile->name) {
    //     options->outfile->file = fopen(options->outfile->name,"w");
    //     if(!options->outfile->file) {
    //         perror(options->outfile->name);
    //         exit(1);
    //     }
    // }

    return 0;
}

int grib_tool_new_filename_action(grib_runtime_options* options, const char* file)
{
    return 0;
}

int grib_tool_new_file_action(grib_runtime_options* options, grib_tools_file* file)
{
    exit_if_input_is_directory(tool_name, file->name);
    return 0;
}

int grib_tool_new_handle_action(grib_runtime_options* options, grib_handle* h)
{
    int err = 0;

    if (!options->skip) {
        if (options->set_values_count != 0)
            err = grib_set_values(h, options->set_values, options->set_values_count);

        if (err != GRIB_SUCCESS && options->fail)
            exit(err);
    }

    if (!options->skip || !options->strict)
        grib_tools_write_message(options, h);

    return 0;
}

int grib_tool_skip_handle(grib_runtime_options* options, grib_handle* h)
{
    grib_handle_delete(h);
    return 0;
}

void grib_tool_print_key_values(grib_runtime_options* options, grib_handle* h)
{
    grib_print_key_values(options, h);
}

int grib_tool_finalise_action(grib_runtime_options* options)
{
    int err = 0;
    grib_file_close_all(&err);
    if (err != GRIB_SUCCESS) {
        perror(tool_name);
        exit(err);
    }

    return 0;
}

int grib_no_handle_action(grib_runtime_options* options, int err)
{
    fprintf(dump_file, "\t\t\"ERROR: unreadable message\"\n");
    return 0;
}
