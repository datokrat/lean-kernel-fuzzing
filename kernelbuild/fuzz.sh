#!/usr/bin/env bash

set -x

# --------  per‑instance environment variables  ---------
ENV0=""                                       # Master instance
ENV1=""
ENV2=""
ENV3=""
ENV4=""
ENV5=""
ENV6="AFL_DISABLE_TRIM=1"
ENV7="AFL_DISABLE_TRIM=1"
ENV8="AFL_DISABLE_TRIM=1"
ENV9="AFL_DISABLE_TRIM=1"
ENV10=""
ENV11=""
ENV12=""
ENV13=""
ENV14=""
ENV15=""
ENV16=""
ENV17=""
ENV18=""
ENV19=""
ENV20="AFL_DISABLE_TRIM=1"
ENV21="AFL_DISABLE_TRIM=1"
ENV22="AFL_DISABLE_TRIM=1"
ENV23="AFL_DISABLE_TRIM=1"
ENV24="AFL_DISABLE_TRIM=1"
ENV25="AFL_DISABLE_TRIM=1"
ENV26="AFL_DISABLE_TRIM=1"
ENV27="AFL_DISABLE_TRIM=1"
ENV28=""
ENV29=""
ENV30=""
ENV31=""

# --------  per‑instance ascii suffixes  ---------
SUFF0=""                                       # Master instance
SUFF1="-asan"
SUFF2="-laf-intel"
SUFF3=""
SUFF4=""
SUFF5=""
SUFF6=""
SUFF7=""
SUFF8=""
SUFF9=""
SUFF10=""
SUFF11=""
SUFF12=""
SUFF13=""
SUFF14=""
SUFF15=""
SUFF16=""
SUFF17=""
SUFF18=""
SUFF19=""
SUFF20=""
SUFF21=""
SUFF22=""
SUFF23=""
SUFF24=""
SUFF25=""
SUFF26=""
SUFF27=""
SUFF28=""
SUFF29=""
SUFF30=""
SUFF31=""

# --------  per‑instance extra afl‑fuzz arguments  ---------
ARG0="-a ascii"                                       # Master instance
ARG1="-a ascii"
ARG2="-a ascii"
ARG3="-a ascii -c ./sql.afl-redqueen"
ARG4="-a ascii -c ./sql.afl-redqueen -l 2AT"
ARG5="-a ascii -P explore -p seek"
ARG6="-P explore -p rare"
ARG7="-a ascii -P exploit"
ARG8="-a ascii -p fast -Z"
ARG9="-a ascii -p coe"
ARG10="-L 0"
ARG11="-L 0"
ARG12="-L 0 -Z"
ARG13="-Z -P explore -p fast "
ARG14="-a ascii"
ARG15="-a ascii"
ARG16="-a ascii -P exploit"
ARG17="-a ascii -P exploit"
ARG18="-a ascii -P explore"
ARG19="-a ascii -P explore"
ARG20="-P explore"
ARG21="-a ascii -P explore -p fast"
ARG22="-P explore -p fast"
ARG23="-a ascii -P explore -p rare"
ARG24="-a ascii -P exploit -p quad"
ARG25="-a ascii -p lin"
ARG26="-a ascii -p coe"
ARG27="-a ascii -p mmopt"
ARG28="-a ascii -p rare"
ARG29="-a ascii -p seek"
ARG30="-a ascii -p fast"
ARG31="-p exploit"

display_help() {
  cat <<EOF
Usage: $0 <number_of_cpu> <input_folder> <output_folder> <binary_name>
   number_of_cpu : number of slave fuzzers to start (max 31)
   input_folder  : seed corpus directory
   output_folder : afl output directory
   binary_name   : target ascii (must be in current dir or give full path)

Per‑instance settings are embedded at the top of this script
(ENV0..ENV31 and ARG0..ARG31).
EOF
  exit 1
}

# basic arg‑count sanity check
[[ $# -lt 4 || $# -gt 4 ]] && display_help

# ---------------- command‑line parsing -----------------
number_of_cpu=$1
input_folder=$2
output_folder=$3
binary_name=$4

# safety clamp: script only honours the first 31 slaves (plus the master)
(( number_of_cpu > 31 )) && number_of_cpu=31

# ---------- helper to fetch per‑instance variables -------
get_var () { local v="$1$2"; printf '%s' "${!v}"; }

# -------------------- launch master ---------------------
env AFL_AUTORESUME=1 $(get_var ENV 0) \
    screen -dmS Main bash -c \
	"afl-fuzz -m 1024 -i \"$input_folder\" -o \"$output_folder\" -M Main $(get_var ARG 0) -- \"$binary_name$(get_var SUFF 0)\""

# -------------------- launch slaves ---------------------
for (( idx=1; idx<=number_of_cpu; idx++ )); do
  env AFL_AUTORESUME=1 $(get_var ENV $idx) \
      screen -dmS "Secondary$idx" bash -c \
	  "afl-fuzz -m 1024 -i \"$input_folder\" -o \"$output_folder\" -S Secondary$idx $(get_var ARG $idx) -- \"$binary_name$(get_var SUFF $idx)\""
done
