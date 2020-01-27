#!/bin/bash -e

# To be called from the top level

BUILD=${1:-build}

git clone https://github.com/DDNStorage/ime_native.git $BUILD/ime-native
git clone https://github.com/DDNStorage/ime_2_posix.git $BUILD/ime-posix

mv $BUILD/ime-native/*.h $BUILD/ime-posix
pushd $BUILD/ime-posix
make
popd

echo "Now you can configure ESDM with:"
echo ./configure --with-ime-include=$PWD/$BUILD/ime-posix/ --with-ime-lib=$PWD/$BUILD/ime-posix/

echo "Configuration information is provided in esdm-ime.conf"
echo '{
	"esdm":	{
		"backends": [
			{
				"type": "IME",
				"id": "p1",
				"target": "./_ime"
			}
		],
		"metadata": {
			"type": "metadummy",
			"id": "md",			"target": "./_metadummy"
		}
	}
}
' > esdm-ime.conf
echo "To test, run:"
echo cp esdm-ime.conf $BUILD/src/test
echo cd $BUILD/src/test
echo ./readwrite-benchmark
