#!/bin/bash -e

# To be called from the top level

git clone https://github.com/DDNStorage/ime_native.git build/ime-native
git clone https://github.com/DDNStorage/ime_2_posix.git build/ime-posix

mv build/ime-native/*.h build/ime-posix
pushd build/ime-posix
make
popd

echo "Now you can configure ESDM with:"
echo ./configure --with-ime-include=$PWD/build/ime-posix/ --with-ime-lib=$PWD/build/ime-posix/

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
echo cp esdm-ime.conf build/src/test
echo cd build/src/test
echo ./readwrite-benchmark
