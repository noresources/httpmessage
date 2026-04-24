#!/usr/bin/env bash

error() {
	echo "error: ${@}" 1>&2
	exit 1
}

warn() {
	echo "warning: ${@}" 1>&2
		return 0
}

step() {
	echo '•' "${@}"
}

# $1 build path
# $2 configuration
# $3 dist path
# $4 Label
build_and_test() {
	c="$(tr '[:upper:]' '[:lower:]' <<< "${2}")"
	step Cleanup "${4}"
	make -C "${1}" config="${c}" clean \
		|| warn Failed to cleanup "${4}"
	step Building "${4}"
	make -C "${1}" config="${c}" \
		|| error Failed to build "${4}"
		tests=($(find "${3}/${2}" -type f -name 'test-*'))
	for t in "${tests[@]}"
	do
		n="$(basename "${t}")"
		step Running "${n}"
		${t} || error "Test ${n} has failed"
	done
}

##############################################################################

###########################

unset configurations
[ ${#} -gt 0 ] \
	&& configurations=("${@}") \
	|| configurations=(Debug Release)

[ -z "${PREMAKE_REPOSITORY}" ] && PREMAKE_REPOSITORY='https://github.com/premake/premake-core.git'
cwd="$(pwd)"
rootPath="$(dirname "${0}")/../.."
cd "${rootPath}" && rootPath="$(pwd)" && cd "${cwd}"

step Finding Premake
unset premake
premake="$(which premake5 2>/dev/null)"

if [ ! -x "${premake}" ]
then
	premakeSourcePath="${rootPath}/premake"
	unset premakeCommit
	if [ ! -d "${premakeSourcePath}" ]
	then
		step Cloning Premake repository
		git clone --quiet "${PREMAKE_REPOSITORY}" "${premakeSourcePath}" \
			|| error Failed to clone premake repository
	fi
	
	if [ -d "${premakeSourcePath}/.git" ]
	then
		premakeCommit="$(git -C "${premakeSourcePath}" log -n 1 --pretty=format:%h)"
			
		git -C "${premakeSourcePath}" fetch --quiet  origin \
			|| warn Failed to fetch Premkae repository
		
		latestPremakeCommit="$(git -C "${premakeSourcePath}" log origin/master -n 1 --pretty=format:%h)"
		
		if [ "${premakeCommit}" != "${latestPremakeCommit}" ]
		then
			step Update Premake working directory from "${premakeCommit}" to "${latestPremakeCommit}"
			
			git -C "${premakeSourcePath}" clean -dxf --quiet \
			&& git -C "${premakeSourcePath}" reset --quiet --hard HEAD \
				|| warn Failed to cleanup premake working directory
			
			git -C "${premakeSourcePath}" pull --quiet \
				|| warn Failed to update premake working directory
		fi
	fi
	
	premake="${premakeSourcePath}/bin/release/premake5"
	
	if [ ! -x "${premake}" ]
	then
		premakeBootstrap="${premakeSourcePath}/Bootstrap.sh"
		[ -x "${premakeBootstrap}" ] \
			|| error Premake build script not found or not executable
		
		step Building Premake
		"${premakeSourcePath}/Bootstrap.sh" 1>/dev/null \
			|| error Failed to build Premake
	fi
	
	[ -x "${premake}" ] \
		|| error "${premake} not found or not executable"
fi


step Building using pre-generated scripts
platform="$(uname -s)"
case "${platform}" in
	Linux)
		platform='linux'
		;;
	Darwin)
		platform='macosx'
		;;
	*)
	;;
esac

buildPath="${rootPath}/scripts/${platform}/gmake"
[ -d "${buildPath}" ] \
	|| error "{platform} platform not supported"

for c in "${configurations[@]}"
do
	build_and_test "${buildPath}" "${c}" "${rootPath}/dist" "${c} configuration using pre-generated build script on ${platform}"
done

for s in Address Thread UndefinedBehavior
do
	location="${rootPath}/build/${s}"
	targetdir="${rootPath}/dist/${s}"
	
	step Generate Premake script with ${s} sanitizer option
	"${premake}" --location="${location}" --targetdir="${targetdir}" \
		--file="${rootPath}/scripts/premake/premake5.lua" \
		--sanitize="${s}" gmake \
		|| error Failed to generate ${s} build scripts
		
	for c in "${configurations[@]}"
	do
		build_and_test "${location}" "${c}" "${targetdir}" "${c} configuration with ${s} sanitizer"
	done
done


