
function readGlobal(){
	var=$(cat src/global_config.h | sed -e "s/#define $1 \(.*\)\$/\\1/" -e 't' -e 'd' | sed "s/\"//g")
	[ -z "${var}" ] && var="0"
	echo $var
	return 0
}

function promptWithDefault(){
	read -p "$1 [$2]: " var
	[ -z "${var}" ] && var="$2"
	echo "$var"
	return 0
}

function promptYesNo(){
	read -p "$1 [y/N] " -n 1 -r
	if [[ $REPLY =~ ^[Yy]$ ]]
	then
		echo
		return 0
	elif [[ $REPLY = "" ]]
	then
		return 1
	else
		echo
		return 1
	fi
}

function copyDataFiles(){
	dest=$1
	shift
	items=($@)
	for file in ${items[@]}
	do
		echo "- bin/data/$file  -> $dest"
		cp -r bin/data/$file "$dest"
	done
}

function getOrCreateWixCode(){
	version="$1"
	var=""
	if [[ -f "assets/wix_db.txt" ]]
	then
		var=$(cat assets/wix_db.txt | sed -e "s|$1=\(.*\)\$|\\1|" -e 't' -e 'd')
	fi
	
	if [ -z "${var}" ]
	then
		var=$(uuidgen)
		addWixCode "$version" "$var"
	fi
	echo $var
	return 0
}

function addWixCode(){
	version="$1"
	code="$2"
	
	echo "$1=$2" >>assets/wix_db.txt
	return 0; 
}