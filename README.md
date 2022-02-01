## Qt Betfair ladder trading application

This application can be used to monitor and place bets on Betfair markets via the Betfair API-NG. It has been developed using the open source development version of Qt Creator 
(version 5.0.1) based on Qt 5.15.2.

### Build Requirements

The open source development version of Qt Creator will need to be installed. Further details can be found [here](https://www.qt.io/download-open-source).

Once installed, this project can be imported via the .pro file. 

This project has been developed on Linux platform (Ubuntu 20.04) but Qt Creator may be installed on other flavours of Linux (as well as Windows and MacOS).

### User Requirements

Users will need to have an active Betfair account and access to the Betfair API. This application makes use of the non-interactive login mechanism 
(described [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Non-Interactive+%28bot%29+login)) which requires and Application key 
and SSL certificate files. 

More information about Application keys can be found [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Application+Keys).

Information about SSL certificates and how to generate them can be found [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Non-Interactive+%28bot%29+login#NonInteractive(bot)login-CreatingaSelfSignedCertificate).

### Current Limitations

1. Only uses API-NG (not streaming)
2. Only capable of viewing one market at a time.
3. No capability of placing bets for amounts lower than the minimum bet. Popular Betfair trading software is capable of this - this application is not.
4. No logging of market data.
5. No bet drag and drop functionality (place/cancel only).
6. Current session Keep-Alive message are NOT sent.
7. Generic event and market browsing is NOT implemented (fixed list of market types for specific event type IDs only).
8. Hard coded limit of 4 price ladders.
