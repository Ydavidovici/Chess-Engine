from pathlib import Path
import importlib.util
from types import ModuleType
from typing import Dict, List


class EngineManager:

    def __init__(self) -> None:
        self.engines: Dict[str, ModuleType] = {}
        self._load_engines()

    def _load_engines(self) -> None:
        root_dir = Path(__file__).resolve().parents[2]
        engines_dir = root_dir / "engines"

        if not engines_dir.exists():
            print(f"[EngineManager] engines directory not found at {engines_dir}")
            return

        print(f"[EngineManager] Scanning for engines in {engines_dir}")

        for so_path in engines_dir.rglob("*.so"):
            full_stem = so_path.stem
            engine_name = full_stem.split(".")[0]

            module_name = engine_name

            print(f"[EngineManager] Attempting to load {so_path} as {module_name}")

            spec = importlib.util.spec_from_file_location(module_name, so_path)
            if spec is None or spec.loader is None:
                print(f"[EngineManager] Failed to create spec for {so_path}")
                continue

            module = importlib.util.module_from_spec(spec)
            try:
                spec.loader.exec_module(module)
            except Exception as e:
                print(f"[EngineManager] Error importing {so_path}: {e}")
                continue

            self.engines[engine_name] = module
            print(f"[EngineManager] Loaded engine '{engine_name}' from {so_path}")

        if not self.engines:
            print("[EngineManager] No engines loaded.")
        else:
            print(f"[EngineManager] Engines loaded: {list(self.engines.keys())}")

    def list_engines(self) -> List[str]:
        return list(self.engines.keys())

    def get_engine_module(self, name: str) -> ModuleType | None:
        return self.engines.get(name)
