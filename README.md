## Qt Betfair ladder trading application

This application can be used to monitor and place bets on Betfair markets via the Betfair API-NG. It has been developed using the open source development version of Qt Creator 
(version 5.0.1) based on Qt 5.15.2.

Please be aware that this application is capable of placing **REAL** bets with the account funds of the logged in user. As a consequence, **ALL** available 
bet placement interfaces are disabled at startup. In addition, the default betting stake is set to the minimum which is £2.
Users will need to manually enable the desired betting interfaces via the "Program Settings" dialog (accessed from the "Settings" menu).

***Additionally, please be aware that this application will display any bets for the user account that have been placed using other software or the main Betfair website 
or mobile apps.***

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

What follows is a very rough overview of how to use the application. Usage is **STRONGLY DISCOURAGED** unless you are an exprienced betting exchange user who is already 
familiar with betting terminology and using ladder based trading interfaces.

#### Stakes

Stake can be selected to be £2, £4, £8, £16, £32, £64, £128, or £256. The stake that will be used is highlighted in **RED**. All stakes are set to minimum (£2) on program startup.

#### Grid View

Left clicking on the price grid will place a back or a lay at the price displayed (plus the optional tick offset if selected) at the stake selected in the stake ladder. 

If the price is a lay price (pink), the bet placed is a lay bet. If the price is a back price (blue), the bet placed is a back bet.

#### Ladder View

##### Ladder Bet Placement

To place a back bet on a selection (at selected stake) at a particular price, click in the appropriate white grid box to the right of the blue coloured column. If the bet is 
unmatched, the stake will appear in the white grid box just clicked. Values in the blue column grids display available money in the market looking to back the runner at that price.

To place a lay bet on a selection (at selected stake) at a particular price, click in the appropriate white grid box to the left of the pink coloured column. If the bet is 
unmatched, the stake will appear in the white grid box just clicked. Values in the pink column grids display available money in the market looking to lay the runner at that price.

##### Ladder Bet Cancellation

Unmatched bets will be displayed in the white ladder columns. To cancel unmatched bets at a specific price, hold down the CONTROL key on the keyboard and then left click on the price. 
To cancel all unmatched bets on a particular side (back or lay) click the "Cancel" button at the top of the appropriate ladder column.


#### Inplay Grid View

Bets will only be placed if the market is currently inplay. Right clicking on any price grid in a row will place a **LAY** bet on that runner at that price (with optional tick 
offset applied if it is selected). Clicking on the "LAY ALL" buttons at the top of a price column will submit **LAY** bets on **ALL** selections at the price shown on the column
 (with tick offset applied if selected).
  
To place a back bet on a selection, hold down the CTRL key on the keyboard and left click at the price. The option to back all runners at a selected price is not supported.


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
* No ladder reversal option.


### Demonstration Videos

- [Inplay trading interface]()
- [Ladder trading interface]()
- [Grid view trading interface]()



