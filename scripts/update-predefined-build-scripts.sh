#!/usr/bin/env bash
cwd="$(pwd)"
root="$(dirname "${0}")/.."
cd "${root}" && root="$(pwd)" && cd "${cwd}"

generate() {
	local action="${1}"
	local os="${2}"
	local location="${root}/scripts/${os}/${action}"
	mkdir -p "${location}" \
	&& premake5 \
	--file="${root}/scripts/premake/premake5.lua" \
		--targetdir="${root}/dist" \
		--location="${location}" \
		--os="${os}" \
		"${action}"
}

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