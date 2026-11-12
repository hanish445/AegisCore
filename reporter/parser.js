const fs = require('fs');

function parseTrace(filePath) {
    try {
        const data = fs.readFileSync(filePath, 'utf8');
        return JSON.parse(data);
    } catch (err) {
        console.error("Error parsing trace file:", err.message);
        return null;
    }
}

module.exports = { parseTrace };