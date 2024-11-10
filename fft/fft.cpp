#include <cmath>
#include <vector>
#include <iostream>
#include <SDL2/SDL.h>

using std::ostream;
using std::vector;

struct Complex {
    double real;
    double imag;
    Complex(): real(0.0), imag(0.0) {}
    Complex(double real, double imag): real(real), imag(imag) {}
    Complex operator+(const Complex& b) {
        return Complex(this->real + b.real, this->imag + b.imag);
    }
    Complex operator-(const Complex& b) {
        return Complex(this->real - b.real, this->imag - b.imag);
    }
    Complex operator*(const double& b) {
        return Complex(this->real * b, this->imag * b);
    }
    Complex operator*(const Complex& b) {
        return Complex(
            this->real * b.real - this->imag * b.imag,
            this->imag * b.real + this->real * b.imag
        );
    }
    friend ostream& operator<<(ostream& stream, const Complex& complex) {
        stream << '(' << complex.real << ", " << complex.imag << "i)";
        return stream; 
    }
    double module() const {
        return std::sqrt(this->real * this->real + this->imag * this->imag);
    };
};

Complex ei(double theta) {
    return Complex(cos(theta), sin(theta));
}

vector<Complex> dft(vector<double> array) {
    size_t N = array.size();
    vector<Complex> result;
    result.reserve(N);
    for (size_t k = 0 ; k < N; k++) {
        Complex sum = Complex();
        for (size_t i = 0; i < N; i++) {
            Complex eitheta = ei(-((2.0 * M_PI) / (double)N) * (double)i * (double)k);
            sum = sum + (eitheta * array[i]);
        }
        result.push_back(sum);
    }
    return result;
}

vector<Complex> fft(const vector<Complex> array) {
    size_t N = array.size();
    vector<Complex> result;
    if (N == 1) {
        result.push_back(ei(0) * array[0]);
        return result;
    };
    result.resize(N, {0.0, 0.0});
    vector<Complex> even, odd;
    for (size_t i = 0; i < N; i++) {
        (i % 2 == 0 ? even : odd).push_back(array[i]);
    }
    vector<Complex> result_even = fft(even);
    vector<Complex> result_odd = fft(odd);
    for (size_t k = 0; k < (N / 2); k++) {
        Complex factor = ei(-(2.0 * M_PI) / (double)N * (double)k);
        result[k] = result_even[k] + result_odd[k] * factor;
        result[k + (N / 2)] = result_even[k] - result_odd[k] * factor;
    }
    return result;
}

vector<Complex> fft(const vector<double> array) {
    size_t N = array.size();
    vector<Complex> complex_array;
    complex_array.reserve(N);
    for (size_t i = 0; i < N; i++)
        complex_array.push_back({ array[i], 0.0 });
    return fft(complex_array);
}

void clear(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
}

void drawCurve(SDL_Renderer* renderer, const vector<double>& result) {
    size_t N = result.size();
    SDL_SetRenderDrawColor(renderer, 191, 191, 191, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, 0, 200, 2048, 200);
    SDL_FPoint* fpoints = new SDL_FPoint[N];
    for (int i = 0; i < N; i++)
        fpoints[i] = { i * 2.0f, -(float)result[i] + 200.0f };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLinesF(renderer, fpoints, N);
    delete[] fpoints;
}

void drawComplex(SDL_Renderer* renderer, const vector<Complex>& result) {
    size_t N = result.size();
    SDL_FPoint* fpoints = new SDL_FPoint[N];

    float max;

    SDL_SetRenderDrawColor(renderer, 191, 191, 191, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, 0, 500, 2048, 500);
    max = 0.1;
    for (int i = 0; i < N; i++)
        if (abs((float)(result[i].module())) > max)
            max = abs((float)(result[i].module()));
    for (int i = 0; i < N; i++)
        fpoints[i] = { i * 2.0f, (-(float)(result[i].module()) / max * 200.0f) + 500.0f };
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLinesF(renderer, fpoints, N);

    SDL_SetRenderDrawColor(renderer, 191, 191, 191, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, 0, 600, 2048, 600);
    max = 0.1;
    for (int i = 0; i < N; i++)
        if (abs((float)(result[i].real)) > max)
            max = abs((float)(result[i].real));
    for (int i = 0; i < N; i++)
        fpoints[i] = { i * 2.0f, (-(float)(result[i].real) / max * 50.0f) + 600.0f };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLinesF(renderer, fpoints, N);

    SDL_SetRenderDrawColor(renderer, 191, 191, 191, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, 0, 700, 2048, 700);
    max = 0.1;
    for (int i = 0; i < N; i++)
        if (abs((float)(result[i].imag)) > max)
            max = abs((float)(result[i].imag));
    for (int i = 0; i < N; i++)
        fpoints[i] = { i * 2.0f, (-(float)(result[i].imag) / max * 50.0f) + 700.0f };
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLinesF(renderer, fpoints, N);

    delete[] fpoints;
}

int main() {
    const size_t SIZE = 1024;
    vector<double> array = vector<double>(SIZE, 0.0);
    for (int i = 0; i < SIZE; i++) {
        
        array[i] += std::sin((double)i * M_PI / 8.00) * 50.0;
        //array[i] += (double)rand() / (double)RAND_MAX * 8.0f - 4.0f;
        
    }
    for (int i = 0; i < SIZE; i++) {
        // array[i] *= (1.0 - sin(std::abs(i - 512) / 512.0 * M_PI_2));    
    }
    auto result = fft(array);
    /*
    for (size_t i = 512 - 480; i < 512 + 480; i++)
        result[i] = {0.0, 0.0};
    */
    //result = fft(result);
    for (int i = 0; i < SIZE; i++) {
        std::cout << "K=" << i << ": " << result[i] << std::endl;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_CreateWindowAndRenderer(2048, 800, SDL_WINDOW_SHOWN, &window, &renderer);

    bool running = true;
    while (running) {
        clear(renderer);
        drawCurve(renderer, array);
        drawComplex(renderer, result);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
                break;
            }
        }
    }
}