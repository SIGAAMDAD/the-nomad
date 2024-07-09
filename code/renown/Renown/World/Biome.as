namespace TheNomad::RenownSystem {
    enum WeatherType {
        Sunny,
        Raining,
        Thunderstorm,
        Foggy,
        Blazing,
        Snowing,
        British, // ;)
    };

    class Biome {
        Biome() {
        }

        const string& GetName() const {
            return m_Name;
        }

        void CycleSeason( uint seasonType ) {

        }

        private string m_Name;
        private WeatherType m_nWeather = WeatherType::Sunny;
    };
};