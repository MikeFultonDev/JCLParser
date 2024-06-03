#!/bin/sh

if [ $# -ne 1 ]; then
  echo "Syntax: $0 <directory>"
  echo "  scan all files in <directory> and report any differences in parsing"
  exit 0
fi

dir="$1"
if ! [ -d "${dir}" ]; then
  echo "${dir} is not a directory. No processing performed" >&2
  exit 4
fi

jcls=$(find "${dir}" -name "*.jcl" -type f)

parsed="/tmp/parsed.jcl"
orig="/tmp/orig.jcl"
tmp="/tmp/tmp.jcl"

if [ "$JCL2JCL_TEST_NOHACKS" != '' ]; then
  nohacks=true
else
  nohacks=false
fi

if [ "$JCL2JCL_COPYFILES" != '' ]; then
  copyfiles=true
else
  copyfiles=false
fi

rc=0
jclfiles=0
parsefail=0
difffail=0

if ${nohacks}; then
   #
   # Run once all defects are fixed and no hacks are
   # remaining.
   # NOT THE DEFAULT!
   #

  for jcl in ${jcls}; do
    jclfiles=$((jclfiles+1))
    ../build/jcl2jcl -i="${jcl}" >"${parsed}"
    if [ $? -gt 0 ]; then
      echo "JCL ${jcl} could not be parsed" >&2
      parsefail=$((parsefail+1))
      rc=1
    else
      diffs=$(diff -b "${jcl}" "${parsed}")
      if [ $? -gt 0 ]; then
        echo "difference for ${jcl}." >&2
        echo "${diffs}" >&2
        difffail=$((difffail+1))
        rc=1
      fi
    fi
  done
  echo "Results (no-hacks):"

else
   #
   # Standard run with hacks in place to work around open defects
   #

  for jcl in ${jcls}; do
    jcl_noext=${jcl%%jcl}
    jclfiles=$((jclfiles+1))
    #Update code as follows:
    # Remove 'comment removal' when #11 is fixed
    # Remove '/* line removal' when #10 is fixed
    # Remove 'cutting' when #16 is fixed
    # Remove 'JOB cleanup' when #9 is fixed
    # Remove 'Generated SYSIN' when #12 is fixed

    grep -v "^//\*" "${jcl}" 2>/dev/null | grep -v "^/\*" 2>/dev/null | cut -c 1-72 >"${orig}" 2>/dev/null
    if [ $? -gt 0 ]; then
      # If there are non-ASCII characters then cut will complain
      # In that case, just use the original jcl
      cp "${jcl}" "${orig}"
    fi
    sed -i x -E "s/JOB[[:space:]]+'.*'/JOB /g" "${orig}" 2>/dev/null
    ../build/jcl2jcl -i="${orig}" >"${parsed}"
    if [ $? -gt 0 ]; then
      echo "JCL ${jcl} could not be parsed" >&2
      if ${copyfiles}; then
        cp "${jcl}" "${jcl_noext}errjcl"
      fi
      parsefail=$((parsefail+1))
      rc=1
    else
      # Remove generated SYSIN lines
      grep -v '//SYSIN  DD \*  (Generated)' "${parsed}" >"${tmp}"
      if [ $? -gt 1 ]; then
        echo "JCL Output file grep failed." >&2
      else 
        cp "${tmp}" "${parsed}"
      fi

      diffs=$(diff -b "${orig}" "${parsed}")
      if [ $? -gt 0 ]; then
        echo "difference for ${jcl}." >&2
        echo "${diffs}" >&2
        if ${copyfiles}; then
          cp "${jcl}" "${jcl_noext}skipjcl"
        fi
        difffail=$((difffail+1))
        rc=1
      else
        if ${copyfiles}; then
          cp "${orig}" "${jcl_noext}modjcl"
        fi
      fi
    fi
  done
  echo "Results (with hacks)"
fi

echo " Directory scanned: ${dir}"
echo " Total files scanned: ${jclfiles}"
echo " Parse failures: ${parsefail}"
echo " Comparison failures: ${difffail}"

exit $rc
