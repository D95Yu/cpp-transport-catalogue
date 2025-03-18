# Транспортный справочник
Данная программа предназначена для хранения информации об автобусах и остановках. Работает с форматами JSON и SVG. По запросу может вывести информацию об остановке, автобусе (его маршруте), рассчитать кратчайший маршрут из точки А в Б. 
## Использование 
__Транспортный справочник__ работает с JSON-форматом: 
* добавляет остановки, автобусы (маршруты), используя "base_requests": 
  ```
  "base_requests": [
    {
        "type": "Stop",
        "name": "Tovarnaya",
        "latitude": 43.587795,
        "longitude": 39.716901,
        "road_distances": {
            "Zapadnaya": 850,
            "Zvyozdnaya": 1740
        }
    },
    {
        "type": "Bus",
        "name": "14",
        "stops": [
            "Zapadnaya",
            "Pushkina",
            "Lenina",
            "Krasniy prospekt",
            "Zvyozdnaya",
            "Universam",
            "Zapadnaya"
        ],
        "is_roundtrip": true
    }  
  ]
  ```
* задает настройки рендера карты, используя "render_settings":
  ```
  "render_settings": {
     "width": 1200,
     "height": 500,
     "padding": 50,
     "stop_radius": 5,
     "line_width": 14,
     "bus_label_font_size": 20,
     "bus_label_offset": [
         7,
         15
     ],
     "stop_label_font_size": 18,
     "stop_label_offset": [
         7,
         -3
     ],
     "underlayer_color": [
         255,
         255,
         255,
         0.85
     ],
     "underlayer_width": 3,
     "color_palette": [
         "green",
         [
             255,
             160,
             0
         ],
         "red"
     ]
  }
  ```
* задает настройки для построения маршрутов, используя "routing_settings":
  ```
  "routing_settings": {
   "bus_wait_time": 2,
   "bus_velocity": 30
  }
  ```
* создает запросы к справочнику, используя "stat_requests":
  ```
  "stat_requests": [
    {
        "id": 218563507,
        "type": "Bus",
        "name": "14"
    },
    {
        "id": 508658276,
        "type": "Stop",
        "name": "Universam"
    },
    {
        "id": 1964680131,
        "type": "Route",
        "from": "Universam",
        "to": "Lenina"
    },
    {
        "id": 1359372752,
        "type": "Map"
    }
  ]
  ```
## Требования
* стандарт C++17 и выше 

