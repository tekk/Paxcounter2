// Decoder for device payload encoder "PACKED"
// copy&paste to TTN Console -> Applications -> PayloadFormat -> Decoder

function Decoder(bytes, port) {

    var decoded = {};

    if (bytes.length === 0)  {
        return {};
    }

    if (port === 1) {
        // only wifi counter data, no gps
        if (bytes.length === 2) {
            return decode(bytes, [uint16], ['wifi']);
        }
        // wifi + ble counter data, no gps
        if (bytes.length === 4) {
            return decode(bytes, [uint16, uint16], ['wifi', 'ble']);
        }
        // combined wifi counter and gps data
        if (bytes.length === 15) {
            return decode(bytes, [uint16, latLng, latLng, uint8, hdop, uint16], ['wifi', 'latitude', 'longitude', 'sats', 'hdop', 'altitude']);
        }
        // combined wifi + ble counter and gps data
        if (bytes.length === 17) {
            return decode(bytes, [uint16, uint16, latLng, latLng, uint8, hdop, uint16], ['wifi', 'ble', 'latitude', 'longitude', 'sats', 'hdop', 'altitude']);
        }
    }

    if (port === 2) {
        // device status data
        return decode(bytes, [uint16, uptime, uint8, uint32, uint8, uint8], ['voltage', 'uptime', 'cputemp', 'memory', 'reset0', 'reset1']);
    }

    if (port === 3) {
        // device config data      
        return decode(bytes, [uint8, uint8, uint16, uint8, uint8, uint8, uint8, bitmap1, bitmap2, version], ['lorasf', 'txpower', 'rssilimit', 'sendcycle', 'wifichancycle', 'blescantime', 'rgblum', 'flags', 'payloadmask', 'version']);
    }

    if (port === 4) {
        // gps data      
        return decode(bytes, [latLng, latLng, uint8, hdop, uint16], ['latitude', 'longitude', 'sats', 'hdop', 'altitude']);
    }

    if (port === 5) {
        // button pressed      
        return decode(bytes, [uint8], ['button']);
    }

    if (port === 6) {
        // beacon proximity alarm      
        return decode(bytes, [uint8, uint8], ['rssi', 'beacon']);
    }

    if (port === 7) {
        // BME680 sensor data     
        return decode(bytes, [float, uint16, ufloat, ufloat], ['temperature', 'pressure', 'humidity', 'air']);
    }

    if (port === 8) {
        // battery voltage      
        return decode(bytes, [uint16], ['voltage']);
    }

}


// ----- contents of /src/decoder.js --------------------------------------------
// https://github.com/thesolarnomad/lora-serialization/blob/master/src/decoder.js

var bytesToInt = function (bytes) {
    var i = 0;
    for (var x = 0; x < bytes.length; x++) {
        i |= (bytes[x] << (x * 8));
    }
    return i;
};

var version = function (bytes) {
    if (bytes.length !== version.BYTES) {
        throw new Error('version must have exactly 10 bytes');
    }
    return String.fromCharCode.apply(null, bytes).split('\u0000')[0];
};
version.BYTES = 10;

var uint8 = function (bytes) {
    if (bytes.length !== uint8.BYTES) {
        throw new Error('uint8 must have exactly 1 byte');
    }
    return bytesToInt(bytes);
};
uint8.BYTES = 1;

var uint16 = function (bytes) {
    if (bytes.length !== uint16.BYTES) {
        throw new Error('uint16 must have exactly 2 bytes');
    }
    return bytesToInt(bytes);
};
uint16.BYTES = 2;

var uint32 = function (bytes) {
    if (bytes.length !== uint32.BYTES) {
        throw new Error('uint32 must have exactly 4 bytes');
    }
    return bytesToInt(bytes);
};
uint32.BYTES = 4;

var latLng = function (bytes) {
    if (bytes.length !== latLng.BYTES) {
        throw new Error('Lat/Long must have exactly 4 bytes');
    }
    return bytesToInt(bytes) / 1e6;
};
latLng.BYTES = 4;

var uptime = function (bytes) {
    if (bytes.length !== uptime.BYTES) {
        throw new Error('Uptime must have exactly 8 bytes');
    }
    return bytesToInt(bytes);
};
uptime.BYTES = 8;

var hdop = function (bytes) {
    if (bytes.length !== hdop.BYTES) {
        throw new Error('hdop must have exactly 2 bytes');
    }
    return bytesToInt(bytes) / 100;
};
hdop.BYTES = 2;

var float = function (bytes) {
    if (bytes.length !== float.BYTES) {
        throw new Error('Float must have exactly 2 bytes');
    }
    var isNegative = bytes[0] & 0x80;
    var b = ('00000000' + Number(bytes[0]).toString(2)).slice(-8)
        + ('00000000' + Number(bytes[1]).toString(2)).slice(-8);
    if (isNegative) {
        var arr = b.split('').map(function (x) { return !Number(x); });
        for (var i = arr.length - 1; i > 0; i--) {
            arr[i] = !arr[i];
            if (arr[i]) {
                break;
            }
        }
        b = arr.map(Number).join('');
    }
    var t = parseInt(b, 2);
    if (isNegative) {
        t = -t;
    }
    return +(t / 100).toFixed(1);
};
float.BYTES = 2;

var ufloat = function (bytes) {
    if (bytes.length !== ufloat.BYTES) {
        throw new Error('Ufloat must have exactly 2 bytes');
    }

    var h = bytesToInt(bytes);
    return +(h / 100).toFixed(1);
};
ufloat.BYTES = 2;

var pressure = function (bytes) {
    if (bytes.length !== pressure.BYTES) {
        throw new Error('Pressure must have exactly 2 bytes');
    }

    var h = bytesToInt(bytes);
    return +(h / 10).toFixed(1);
};
pressure.BYTES = 2;

var bitmap1 = function (byte) {
    if (byte.length !== bitmap1.BYTES) {
        throw new Error('Bitmap must have exactly 1 byte');
    }
    var i = bytesToInt(byte);
    var bm = ('00000000' + Number(i).toString(2)).substr(-8).split('').map(Number).map(Boolean);
    return ['adr', 'screensaver', 'screen', 'countermode', 'blescan', 'antenna', 'filter', 'alarm']
        .reduce(function (obj, pos, index) {
            obj[pos] = +bm[index];
            return obj;
        }, {});
};
bitmap1.BYTES = 1;

var bitmap2 = function (byte) {
    if (byte.length !== bitmap2.BYTES) {
        throw new Error('Bitmap must have exactly 1 byte');
    }
    var i = bytesToInt(byte);
    var bm = ('00000000' + Number(i).toString(2)).substr(-8).split('').map(Number).map(Boolean);
    return ['gps', 'alarm', 'bme', 'counter', 'sensor1', 'sensor2', 'sensor3', 'battery']
        .reduce(function (obj, pos, index) {
            obj[pos] = +bm[index];
            return obj;
        }, {});
};
bitmap2.BYTES = 1;

var decode = function (bytes, mask, names) {

    var maskLength = mask.reduce(function (prev, cur) {
        return prev + cur.BYTES;
    }, 0);
    if (bytes.length < maskLength) {
        throw new Error('Mask length is ' + maskLength + ' whereas input is ' + bytes.length);
    }

    names = names || [];
    var offset = 0;
    return mask
        .map(function (decodeFn) {
            var current = bytes.slice(offset, offset += decodeFn.BYTES);
            return decodeFn(current);
        })
        .reduce(function (prev, cur, idx) {
            prev[names[idx] || idx] = cur;
            return prev;
        }, {});
};

if (typeof module === 'object' && typeof module.exports !== 'undefined') {
    module.exports = {
        uint8: uint8,
        uint16: uint16,
        uint32: uint32,
        uptime: uptime,
        float: float,
        ufloat: ufloat,
        pressure: pressure,
        latLng: latLng,
        hdop: hdop,
        bitmap1: bitmap1,
        bitmap2: bitmap2,
        version: version,
        decode: decode
    };
}