#!/usr/bin/env bash
cwd="$(pwd)"
root="$(dirname "${0}")/.."
cd "${root}" && root="$(pwd)" && cd "${cwd}"

generate() {
	local action="${1}"
	local os="${2}"
	
	local actiondir="$(cut -f 2 -d':' <<< "${action}")"
	local action="$(cut -f 1 -d':' <<< "${action}")"
	
	[ -z "${actiondir}" ] \
		&& actiondir="${action}"
	
	local options=(\
		--file="${root}/scripts/premake/premake5.lua" \
		--targetdir="${root}/dist" \
	)
	[ -z "${os}" ] \
		&& os='.' \
		|| options=("${options[@]}" --os="${os}")
	
	local location="${root}/scripts/${os}/${actiondir}"
	options=("${options[@]}" --location="${location}")
	
	mkdir -p "${location}" \
	&& premake5 \
		"${options[@]}" \
		"${action}"
}

unset os
for action in preprocess:doxygen doxygen
do
	generate "${action}"  
done

os=linux
for action in gmake2
do
	generate "${action}" "${os}"  
done

os=macosx
for action in gmake2 xcode4
do
	generate "${action}" "${os}"  
done

os=windows
for action in gmake2 vs2017
do
	generate "${action}" "${os}"  
done