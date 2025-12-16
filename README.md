### Commands to execute the program :-

We can run these from the root directory only.

```bash
rm -rf build
```

```bash
cmake -G "Ninja" -B build; ninja -C build;
```

```bash
./build/stencilc test/simple.toy
```

### Commands for graph generation :-

-   First ensure `venv` is there and `requirements.txt` has been installed.

```bash
python -m venv venv
```

```bash
pip install -r requirements.txt
```

Next execute these commands :-

```bash
bash bench/run_bench.sh
```

```bash
python3 bench/parse_results.py
```

```bash
python3 bench/plot_with_pandas.py
```

-   Additional info for reference is present at [Output-README.md](./bench/README.md)
