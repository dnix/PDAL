FROM alpine:edge

ADD . /pdal

RUN \
    echo "http://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories; \
    apk update; \
    apk add --no-cache --virtual .build-deps \
        alpine-sdk \
        eigen-dev \
        hexer-dev \
        nitro-dev \
        gdal-dev \
        geos-dev \
        laz-perf-dev \
        libgeotiff-dev \
        libxml2-dev \
        python-dev \
        py-numpy-dev \
        jsoncpp-dev \
        hdf5-dev \
        proj4-dev \
        cpd-dev \
        fgt-dev \
        sqlite-dev \
        postgresql-dev \
        curl-dev \
        linux-headers \
    ; \
    apk add --no-cache \
        cmake \
        hexer \
        nitro \
        gdal \
        geos \
        laz-perf \
        libgeotiff \
        libxml2 \
        python \
        py-numpy \
        jsconcpp \
        hdf5 \
        proj4 \
        cpd \
        fgt \
        sqlite \
        postgresql \
        libcurl \
    ;\
    cd /pdal; \
    mkdir -p _build; \
    cd _build; \
    cmake .. \
        -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=gcc \
        -DCMAKE_CXX_COMPILER=g++ \
        -DCMAKE_MAKE_PROGRAM=make \
        -DBUILD_PLUGIN_PYTHON=ON \
        -DBUILD_PLUGIN_CPD=ON \
        -DBUILD_PLUGIN_GREYHOUND=ON \
        -DBUILD_PLUGIN_HEXBIN=ON \
        -DBUILD_PLUGIN_NITF=ON \
        -DBUILD_PLUGIN_ICEBRIDGE=ON \
        -DBUILD_PLUGIN_PGPOINTCLOUD=ON \
        -DWITH_LASZIP=ON \
        -DWITH_LAZPERF=ON \
    ; \
    make -j2; \
    make install; \
    cd /; \
    rm -rf pdal; \
    apk del .build-deps

CMD ["pdal"]
