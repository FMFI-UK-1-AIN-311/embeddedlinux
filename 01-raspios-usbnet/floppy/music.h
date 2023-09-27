#include <vector>
#include <unordered_map>
struct Note { int freq; int duration; };
using Song = std::vector<Note>;

extern const Song DoReMi;
extern const Song Scale;

extern const Song CervenyKachel;
extern const Song Cizicek;
extern const Song Kohutik;
extern const Song KrasnaKrasna;
extern const Song Kukulienka;
extern const Song MalickaSom;
extern const Song MaliMuzikanti;
extern const Song Medvedku;
extern const Song MojaMamka;
extern const Song NaZelenejLuke;
extern const Song PojdemeMyDoHajicka;
extern const Song PrsiPrsi;
extern const Song SedemdesiatSukien;
extern const Song SosovickaHrasokFazula;
extern const Song SpiDietatkoSpize;
extern const Song Spievanky;
extern const Song VstavajHonzo;
extern const Song ZlataBrana;
extern const Song ImperialMarch;

//extern const std::vector<Song*> AllSongs;
extern const std::vector<const Song*> AllSongs;
