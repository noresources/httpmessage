#!/usr/bin/env bash
cwd="$(pwd)"
root="$(dirname "${0}")/../.."
cd "${root}" && root="$(pwd)" && cd "${cwd}"

os="${1}"
[ -z "${os}" ] && os="$(uname -s)"
os="$(tr '[:upper:]' '[:lower:]' <<< "${os}")"
[ "${os}" = 'darwin' ] && os='macosx'

exitCode=0

for tool in \
	address \
	cfi \
	dataflow \
	memory \
	safe-stack \
	thread \
	undefined
do
		echo "-- ${tool} ------------------------------"

		cflags=( \
			-fsanitize=${tool} \
		)
		ldflags=( \
			-fsanitize=${tool} \
		)
		
		case "${tool}" in
			cfi)
				cflags=("${cflags[@]}" \
					-fvisibility= \
					-flto=full)
			;;
		esac
		
		clang ${cflags[*]} \
			-x c -E '' - </dev/null  1>/dev/null \
			|| continue
		
		make -C "${root}/scripts/${os}/gmake2" \
			clean \
			1>/dev/null
		
		result=$?
		[ ${result} -ne 0 ] \
			&& exitCode=$((${exitCode} + 1)) \
			&& continue
			
		make -C "${root}/scripts/${os}/gmake2" \
			CFLAGS="${cflags[*]}" \
			LDFLAGS="${ldflags[*]}" \
			CC=clang \
			1>/dev/null
		result=$?
		
		if [ ${result} -ne 0 ]
		then
			echo "Failed to build with ${tool} sanitizer" 1>&2
			exitCode=$((${exitCode} + 1))
			continue
		fi
			
	for test in \
		text \
		headers \
		messages
	do
		echo '----' "${test}" '----------------------------'
		"${root}/dist/Debug/tests/test-${test}" \
			1>/dev/null
		
		result=$?
		
		if [ ${result} -ne 0 ]
		then
			echo "${tool} sanitizer failure for ${test} test" 1>&2
			exitCode=$((${exitCode} + 1))
		fi
	done
done

exit ${exitCode}