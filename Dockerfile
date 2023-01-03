FROM ubuntu:20.04 as build-deps

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -yq \
    git \
    build-essential \
    devscripts \
    build-essential \
    lintian \
    debhelper \
    libgtk2.0-dev \
    libxml2-dev \
    libcurl3-dev \
    libcurl4-openssl-dev \
    libmicrohttpd-dev \
    libqrencode-dev \
    libssl-dev \
    libjson-glib-dev \
    libgoocanvas-2.0-dev \
    libzip-dev \
    libwebkit2gtk-4.0-dev \
    libusb-1.0-0-dev \
    libwebsockets-dev


FROM build-deps as build

WORKDIR /build

COPY . .

RUN echo '#define TWITTER_CONSUMER_KEY "twitter-key"' >> ./sources/common/network/advertiser_ids.hpp && \
    echo '#define TWITTER_CONSUMER_SECRET "twitter-secret"'  >> ./sources/common/network/advertiser_ids.hpp && \
    echo '#define FACEBOOK_CONSUMER_KEY "facebook-key"'  >> ./sources/common/network/advertiser_ids.hpp && \
    echo '#define FACEBOOK_CONSUMER_SECRET "facebook-secret"'  >> ./sources/common/network/advertiser_ids.hpp && \
    cd build/BellePoule && \
    make -s DISTRIB=focal V=1 package && \
    cd ~/Perso/PPA/bellepoulebeta/bellepoulebeta_6.0 && \    
    make -s build && \
    cd .. && \
    cp $(ls *.deb) build.deb



FROM ubuntu:20.04 as runtime

ARG DEBIAN_FRONTEND=noninteractive

RUN apt update && apt upgrade -yq && \
    apt install -yq \
    php-xml \
    php \
    gamin \
    libzip5 \
    libxml2 \
    libwebsockets15 \
    libusb-1.0-0 \
    libssl1.1 \
    libqrencode4 \
    libpangocairo-1.0-0 \
    libpango-1.0-0 \
    libmicrohttpd12 \
    libjson-glib-1.0-0 \
    libgtk2.0-0 \
    libgdk-pixbuf2.0-0 \
    libcurl4 \
    libcairo2 \
    libatk1.0-0 \
    php-cli

WORKDIR /root/

COPY --from=build /root/Perso/PPA/bellepoulebeta/build.deb .

RUN ln -s /usr/bin/php7.4 /usr/bin/php7.0 && \
    dpkg --install ./build.deb

CMD bellepoulebeta-supervisor
