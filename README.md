## Qt Betfair ladder trading application

This application can be used to monitor and place bets on Betfair markets via the Betfair API-NG. It has been developed using the open source development version of Qt Creator 
(version 5.0.1) based on Qt 5.15.2.

Please be aware that this application is capable of placing **real** bets with the account funds of the logged in user. As a consequence, **all** of the available 
bet placement interfaces are disabled at startup. In addition, the default betting stake is set to the minimum which is £2.
Users will need to manually enable the desired betting interfaces via the "Program Settings" dialog (accessed from the "Settings" menu).

#### Table of Contents
- [Build Requirements](#build-requirements)
- [User Requirements](#user-requirements)
- [License](#license)
- [Usage Instructions](#usage-instructions)
- [Current Limitations](#current-limitations)
- [Demonstration Videos](#demonstration-videos)


### Build Requirements

This application has been developed with the open source development version of Qt Creator which will need to be installed on your system.
Further details about open source development with Qt and instructions for dowloading and installing Qt Creator can be found [here](https://www.qt.io/download-open-source).
This project has been developed on Ubuntu Linux (LTS version 20.04) but Qt Creator can be installed on Windows and MacOS, as well as other Linux flavours.

Once Qt Creator is installed, the project source code can be imported via the .pro file. 


### User Requirements

Users will need to have an active Betfair account and access to the Betfair API. This application makes use of the non-interactive login mechanism 
(described [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Non-Interactive+%28bot%29+login)) which requires an application key 
and SSL certificate files. 

More information about application keys can be found [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Application+Keys).

Information about SSL certificates and how to generate them can be found [here](https://docs.developer.betfair.com/display/1smk3cen4v3lu3yomq5qye0ni/Non-Interactive+%28bot%29+login#NonInteractive(bot)login-CreatingaSelfSignedCertificate).


### License

Distributed under the MIT License. See [LICENSE](https://github.com/doctorcee/betfair_ladder_trader/blob/main/LICENSE) for more information.


## Usage Instructions

#### Current Limitations

* Only uses API-NG (not the streaming API)
* Only capable of viewing one market at a time.
* No capability of placing bets for amounts lower than the minimum bet (popular commercial Betfair trading software is capable of this - this application is not).
* No logging of market data.
* No ladder bet drag and drop functionality (place/cancel only).
* Current session Keep-Alive message are **NOT** sent.
* Generic event and market browsing is NOT implemented (fixed list of market types for specific event type IDs only).
* Hard coded limit of 4 price ladders.
* No support for LINE or ASIAN_HANDICAP betting type markets.
* No facility to bet at Betfair SP.
* Hard coded stakes of £2, £4, £8, £16, £32, £64, £128, £256 are current stakes available (does not apply to hedged positions)
* Hedging only applies to the **SELECTION**. No full market greening up option yet.
* Lay bet stakes represent the amount that would be ***WON*** and ***NOT*** the actual liability if the bet loses.



### Demonstration Videos

- [Inplay trading interface]()
- [Ladder trading interface]()
- [Grid view trading interface]()



