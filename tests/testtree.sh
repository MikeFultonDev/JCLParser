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
tmp="/tmp/tmp.jcl"
for jcl in ${jcls}; do
  #Remove all comments and cut everything past column 72
  #Update code as follows:
  # Remove 'comment removal' when #11 is fixed
  # Remove '/* line removal' when #10 is fixed
  # Remove 'cutting' when #16 is fixed
  # Remove 'JOB cleanup' when #9 is fixed
  # Remove 'Generated SYSIN' when #12 is fixed

  grep -v "^//\*" "${jcl}" | grep -v "^/\*" | cut -c 1-72 >"${orig}" 2>/dev/null
  if [ $? -gt 0 ]; then
    # If there are non-ASCII characters then cut will complain
    # In that case, just use the original jcl
    cp "${jcl}" "${orig}"
  fi
  sed -i x -E "s/JOB[[:space:]]+'.*'/JOB /g" "${orig}"
  ../build/jcl2jcl -i="${orig}" >"${parsed}"
  if [ $? -gt 0 ]; then
    echo "JCL ${jcl} could not be parsed" >&2
  else
    # Remove generated SYSIN lines
    grep -v '//SYSIN  DD \*  (Generated)' "${parsed}" >"${tmp}"
    cp "${tmp}" "${parsed}"

    diffs=$(diff -b "${orig}" "${parsed}")
    if [ $? -gt 0 ]; then
      echo "difference for ${jcl}." >&2
      echo "${diffs}" >&2
    fi
  fi
done

