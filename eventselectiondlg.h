#ifndef EVENTSELECTIONDLG_H
#define EVENTSELECTIONDLG_H

#include <QDialog>

namespace Ui {
class TEventSelectionDlg;
}

class TEventSelectionDlg : public QDialog
{
    Q_OBJECT

public:
    explicit TEventSelectionDlg(QWidget *parent = nullptr);
    ~TEventSelectionDlg();

    QString getMarketFilter() const {return m_market_filter;}

public slots:
    void onLoadTodaysUKHorsesWIN();
    void onLoadTodaysUKIREHorsesWIN();
    void onLoadDogMarkets();
    void onLoadCustomHorseMarkets();
    void onLoadCustomFootballMarkets();

signals:
    void selectedEventChanged();

private:
    Ui::TEventSelectionDlg *ui;

    QString m_current_loaded_event;
    QString m_market_filter;

    void createUKFootballMarketFilter(const std::vector<QString>& competition_ids,
                                      const std::vector<QString>& market_types,
                                      const QDate& sel_day);
};

#endif // EVENTSELECTIONDLG_H
