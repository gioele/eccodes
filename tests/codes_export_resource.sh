#!/bin/sh
# (C) Copyright 2005- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# 
# In applying this licence, ECMWF does not waive the privileges and immunities granted to it by
# virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
#

. ./include.sh

label="codes_export_resource_test"
temp=temp.$label

# Sample files
# --------------
f='GRIB2.tmpl'
${tools_dir}/codes_export_resource -s $f $temp
cmp $ECCODES_SAMPLES_PATH/$f $temp

# Use the shortened form
${tools_dir}/codes_export_resource -s GRIB2 $temp
cmp $ECCODES_SAMPLES_PATH/GRIB2.tmpl $temp

# Definition files
# ----------------
f='boot.def'
${tools_dir}/codes_export_resource -d $f $temp
cmp $ECCODES_DEFINITION_PATH/$f $temp

f='common/c-11.table'
${tools_dir}/codes_export_resource -d $f $temp
cmp $ECCODES_DEFINITION_PATH/$f $temp


# Clean up
rm -f $temp
