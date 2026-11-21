#include "calc.h"   // Declares runEngineeringCalculator()
#include <algorithm>// Provides sort for median calculation
#include <cmath>    // Provides sin, cos, tan, sqrt, pow, fabs
#include <complex>  // Provides complex<double> for complex equation arithmetic
#include <iomanip>  // Provides fixed, setprecision, setw, and right formatting
#include <iostream> // Provides cin and cout for console I/O
#include <limits>   // Provides numeric_limits for input recovery
#include <vector>   // Provides vector for statistics arrays

using namespace std; // Brings std names into calc.cpp

namespace { // Private math helpers for calc.cpp

constexpr double PI = 3.14159265358979323846; // Mathematical constant pi

// ENGINEERING CALCULATOR: Converts degrees to radians for trig functions
double degToRad(double deg) {
    return deg * (PI / 180.0);
}

// ENGINEERING CALCULATOR: Unit conversion submenu (Length, Mass, Temperature, Pressure)
void unitConversionMenu() {
    int type = 0;
    cout << "\n--- Unit Conversions ---\n";
    cout << "[1] Meters <-> Feet\n";
    cout << "[2] Kilograms <-> Pounds\n";
    cout << "[3] Celsius <-> Fahrenheit\n";
    cout << "[4] Kilopascals (kPa) <-> PSI\n";
    cout << "Select: ";
    if (!(cin >> type)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    double val = 0.0;
    cout << "Enter value: ";
    if (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << fixed << setprecision(4);
    if (type == 1) {
        cout << val << " m = " << (val * 3.28084) << " ft | "
             << val << " ft = " << (val / 3.28084) << " m\n";
    } else if (type == 2) {
        cout << val << " kg = " << (val * 2.20462) << " lb | "
             << val << " lb = " << (val / 2.20462) << " kg\n";
    } else if (type == 3) {
        cout << val << " C = " << ((val * 9.0 / 5.0) + 32.0) << " F | "
             << val << " F = " << ((val - 32.0) * 5.0 / 9.0) << " C\n";
    } else if (type == 4) {
        cout << val << " kPa = " << (val * 0.145038) << " PSI | "
             << val << " PSI = " << (val / 0.145038) << " kPa\n";
    } else {
        cout << "Invalid conversion choice.\n";
    }
}

// ENGINEERING CALCULATOR: Trigonometry calculator in degrees
void trigMenu() {
    double deg = 0.0;
    cout << "\n--- Trigonometry (Input in Degrees) ---\n";
    cout << "Angle (deg): ";
    if (!(cin >> deg)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    double rad = degToRad(deg);
    cout << fixed << setprecision(4);
    cout << "sin(" << deg << ") = " << sin(rad) << "\n";
    cout << "cos(" << deg << ") = " << cos(rad) << "\n";
    if (fabs(cos(rad)) < 1e-9) {
        cout << "tan(" << deg << ") = Undefined (cos near 0)\n";
    } else {
        cout << "tan(" << deg << ") = " << tan(rad) << "\n";
    }
}

// ENGINEERING CALCULATOR: Descriptive statistics on a numeric dataset
void statsMenu() {
    int n = 0;
    cout << "\n--- Statistics Calculator ---\n";
    cout << "How many numbers? ";
    if (!(cin >> n) || n <= 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter a positive count.\n";
        return;
    }

    vector<double> nums(n);
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        cout << "Value #" << (i + 1) << ": ";
        cin >> nums[i];
        sum += nums[i];
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    double mean = sum / n;
    double sqDiffSum = 0.0;
    for (double x : nums) {
        sqDiffSum += pow(x - mean, 2);
    }
    double variance = (n > 1) ? (sqDiffSum / (n - 1)) : 0.0;
    double stdDev = sqrt(variance);

    vector<double> sorted = nums;
    sort(sorted.begin(), sorted.end());
    double median = (n % 2 == 1)
                        ? sorted[n / 2]
                        : (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;

    cout << fixed << setprecision(4);
    cout << left << setw(18) << "Count:" << right << setw(12) << n << "\n";
    cout << left << setw(18) << "Mean:" << right << setw(12) << mean << "\n";
    cout << left << setw(18) << "Median:" << right << setw(12) << median << "\n";
    cout << left << setw(18) << "Min:" << right << setw(12) << sorted.front() << "\n";
    cout << left << setw(18) << "Max:" << right << setw(12) << sorted.back() << "\n";
    cout << left << setw(18) << "Std Deviation:" << right << setw(12) << stdDev << "\n";
}

// ENGINEERING CALCULATOR: 2x2 matrix addition and determinant solver
void matrixMenu() {
    double a[2][2], b[2][2];
    cout << "\n--- 2x2 Matrix Operations ---\n";
    cout << "Enter Matrix A (4 values row-by-row): ";
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 2; ++c)
            cin >> a[r][c];

    cout << "Enter Matrix B (4 values row-by-row): ";
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 2; ++c)
            cin >> b[r][c];
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << fixed << setprecision(2);
    cout << "\nMatrix A + B:\n";
    for (int r = 0; r < 2; ++r) {
        cout << "[ " << setw(8) << (a[r][0] + b[r][0])
             << " " << setw(8) << (a[r][1] + b[r][1]) << " ]\n";
    }

    double detA = (a[0][0] * a[1][1]) - (a[0][1] * a[1][0]);
    cout << "Determinant of A: " << detA << "\n";
}

// ENGINEERING CALCULATOR: Solves ax^2 + bx + c = 0 (real & complex roots) & formula helper
void equationAndComplexMenu() {
    int mode = 0;
    cout << "\n--- Equations, Formulas & Complex Numbers ---\n";
    cout << "[1] Quadratic Solver (ax^2 + bx + c = 0)\n";
    cout << "[2] Complex Number Add/Multiply (a+bi, c+di)\n";
    cout << "[3] Ohm's Law Solver (V = I * R)\n";
    cout << "Select: ";
    if (!(cin >> mode)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << fixed << setprecision(4);
    if (mode == 1) {
        double a, b, c;
        cout << "Enter a, b, c: ";
        cin >> a >> b >> c;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (fabs(a) < 1e-9) {
            cout << "Not quadratic (a = 0).\n";
            return;
        }
        double disc = b * b - 4.0 * a * c;
        if (disc >= 0.0) {
            double r1 = (-b + sqrt(disc)) / (2.0 * a);
            double r2 = (-b - sqrt(disc)) / (2.0 * a);
            cout << "Real roots: x1 = " << r1 << ", x2 = " << r2 << "\n";
        } else {
            double realPart = -b / (2.0 * a);
            double imagPart = sqrt(-disc) / (2.0 * a);
            cout << "Complex roots: x1 = " << realPart << " + " << imagPart << "i, "
                 << "x2 = " << realPart << " - " << imagPart << "i\n";
        }
    } else if (mode == 2) {
        double r1, i1, r2, i2;
        cout << "Enter z1 real and imag: ";
        cin >> r1 >> i1;
        cout << "Enter z2 real and imag: ";
        cin >> r2 >> i2;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        complex<double> z1(r1, i1), z2(r2, i2);
        cout << "Sum:     " << (z1 + z2) << "\n";
        cout << "Product: " << (z1 * z2) << "\n";
    } else if (mode == 3) {
        double iVal, rVal;
        cout << "Enter Current I (Amps) and Resistance R (Ohms): ";
        cin >> iVal >> rVal;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Voltage V = " << (iVal * rVal) << " Volts\n";
    } else {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

} // namespace

// ENGINEERING CALCULATOR: Main submenu loop for all calculation modes
void runEngineeringCalculator() {
    int choice = -1;
    while (choice != 0) {
        cout << "\n--- ADVANCED ENGINEERING CALCULATOR ---\n";
        cout << "[1] Unit Conversions\n";
        cout << "[2] Trigonometry\n";
        cout << "[3] Statistics\n";
        cout << "[4] Matrices (2x2)\n";
        cout << "[5] Equations, Complex Numbers & Formulas\n";
        cout << "[0] Back to Dashboard\n";
        cout << "Choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = -1;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: unitConversionMenu(); break;
            case 2: trigMenu(); break;
            case 3: statsMenu(); break;
            case 4: matrixMenu(); break;
            case 5: equationAndComplexMenu(); break;
            case 0: break;
            default: cout << "Invalid choice.\n"; break;
        }
    }
}
