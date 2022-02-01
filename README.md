## Qt Betfair ladder trading application

This application can be used to monitor and place bets on Betfair markets via the Betfair API-NG. It has been developed using the open source development version of Qt Creator 
(version 5.0.1) based on Qt 5.15.2.

***Because this application is capable of placing REAL bets using the logged in users account funds, bet placement functionality for ALL of the available bet placement 
interfaces is disabled at startup. Users will need to manually enable the desired betting interfaces via the "Program Settings" dialog (accessed from the "Settings" menu).*** 

### Build Requirements

No installation scripts are provided and the open source development version of Qt Creator will need to be installed to compile the source code. 
Further details about open source development using Qt can be found [here](https://www.qt.io/download-open-source).

Once Qt Creator is installed, the project source code can be imported via the .pro file. 

This project has been developed on Linux platform (Ubuntu 20.04) but Qt Creator may be installed on other flavours of Linux (as well as Windows and MacOS).

### User Requirements

Users will need to have an active Betfair account and access to the Betfair API. This application makes use of the non-interactive login mechanism 
(described [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Non-Interactive+%28bot%29+login)) which requires an application key 
and SSL certificate files. 

More information about application keys can be found [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Application+Keys).

Information about SSL certificates and how to generate them can be found [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Non-Interactive+%28bot%29+login#NonInteractive(bot)login-CreatingaSelfSignedCertificate).

## License

Distributed under the MIT License. See [LICENSE](https://github.com/doctorcee/betfair_ladder_trader/blob/main/LICENSE) for more information.

### Current Limitations

* Only uses API-NG (not the streaming API)
* Only capable of viewing one market at a time.
* No capability of placing bets for amounts lower than the minimum bet (popular commercial Betfair trading software is capable of this - this application is not).
* No logging of market data.
* No ladder bet drag and drop functionality (place/cancel only).
* Current session Keep-Alive message are NOT sent.
* Generic event and market browsing is NOT implemented (fixed list of market types for specific event type IDs only).
* Hard coded limit of 4 price ladders.
* No support for LINE or ASIAN_HANDICAP betting type markets.
