#!/bin/sh

if [ $# -ne 1 ]; then
  echo "Syntax: $0 <directory>"
  echo "  scan all files in <directory> and report any differences in parsing"
  exit 0
fi

dir="$1"
if ! [ -d "${dir}" ]; then
  echo "${dir} is not a directory. No processing performed"
  exit 4
fi

jcls=$(find "${dir}" -name "*.jcl" -type f)

parsed="/tmp/parsed.jcl"
orig="/tmp/orig.jcl"
for jcl in ${jcls}; do
  ../build/jcl2jcl -i="${jcl}" >"${parsed}"
  if [ $? -gt 0 ]; then
    echo "JCL ${jcl} could not be parsed" >&2
  else
    cat "${jcl}" | cut -c 1-72 >"${orig}"
    diffs=$(diff -b "${orig}" "${parsed}")
    if [ $? -gt 0 ]; then
      echo "difference for ${jcl}." >&2
      echo "${diffs}" >&2
    fi
  fi
done

