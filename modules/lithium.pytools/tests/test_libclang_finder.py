import pytest
from unittest.mock import patch, MagicMock
from pathlib import Path
from ..libclang_finder import LibClangFinder, LibClangFinderConfig

# FILE: modules/lithium.pytools/tools/test_libclang_finder.py


@pytest.fixture
def libclang_finder_config():
    # Provide a default LibClangFinderConfig instance
    return LibClangFinderConfig()


@pytest.fixture
def libclang_finder_instance(libclang_finder_config):
    # Provide a LibClangFinder instance initialized with the default configuration
    return LibClangFinder(config=libclang_finder_config)


def test_initialization_default():
    config = LibClangFinderConfig()
    finder = LibClangFinder(config)
    assert finder.config.method == 'bilinear'
    assert finder.config.pattern is None
    assert finder.config.num_threads == 4
    assert not finder.config.visualize_intermediate
    assert finder.config.visualization_save_path is None
    assert not finder.config.save_debayered_images


def test_initialization_custom():
    config = LibClangFinderConfig(custom_path=Path(
        '/custom/path/to/libclang.so'), clear_cache=True)
    finder = LibClangFinder(config)
    assert finder.config.custom_path == Path('/custom/path/to/libclang.so')
    assert finder.config.clear_cache is True


def test_clear_cache(libclang_finder_instance):
    with patch('pathlib.Path.unlink') as mock_unlink:
        libclang_finder_instance.clear_cache()
        mock_unlink.assert_called_once()


def test_cache_libclang_path(libclang_finder_instance):
    path = Path('/path/to/libclang.so')
    with patch('pathlib.Path.write_text') as mock_write_text:
        libclang_finder_instance.cache_libclang_path(path)
        mock_write_text.assert_called_once_with(str(path))


def test_load_cached_libclang_path(libclang_finder_instance):
    path = Path('/path/to/libclang.so')
    with patch('pathlib.Path.exists', return_value=True), \
            patch('pathlib.Path.read_text', return_value=str(path)), \
            patch('pathlib.Path.is_file', return_value=True):
        cached_path = libclang_finder_instance.load_cached_libclang_path()
        assert cached_path == path


def test_find_libclang_linux(libclang_finder_instance):
    with patch('glob.glob', return_value=['/usr/lib/llvm-18/lib/libclang.so']), \
            patch('pathlib.Path.is_file', return_value=True):
        path = libclang_finder_instance.find_libclang_linux()
        assert path == Path('/usr/lib/llvm-18/lib/libclang.so')


def test_find_libclang_macos(libclang_finder_instance):
    with patch('glob.glob', return_value=['/usr/local/opt/llvm/lib/libclang.dylib']), \
            patch('pathlib.Path.is_file', return_value=True):
        path = libclang_finder_instance.find_libclang_macos()
        assert path == Path('/usr/local/opt/llvm/lib/libclang.dylib')


def test_find_libclang_windows(libclang_finder_instance):
    with patch('glob.glob', return_value=['C:\\Program Files\\LLVM\\bin\\libclang.dll']), \
            patch('pathlib.Path.is_file', return_value=True):
        path = libclang_finder_instance.find_libclang_windows()
        assert path == Path('C:\\Program Files\\LLVM\\bin\\libclang.dll')


def test_search_paths(libclang_finder_instance):
    patterns = ['/usr/lib/llvm-*/lib/libclang.so*']
    with patch('glob.glob', return_value=['/usr/lib/llvm-18/lib/libclang.so']), \
            patch('pathlib.Path.is_file', return_value=True):
        paths = libclang_finder_instance.search_paths(patterns)
        assert paths == [Path('/usr/lib/llvm-18/lib/libclang.so')]


def test_select_libclang_path(libclang_finder_instance):
    paths = [Path('/usr/lib/llvm-18/lib/libclang.so')]
    selected_path = libclang_finder_instance.select_libclang_path(paths)
    assert selected_path == Path('/usr/lib/llvm-18/lib/libclang.so')


def test_get_libclang_path_with_custom_path(libclang_finder_instance):
    libclang_finder_instance.config.custom_path = Path(
        '/custom/path/to/libclang.so')
    with patch('pathlib.Path.is_file', return_value=True), \
            patch.object(libclang_finder_instance, 'cache_libclang_path') as mock_cache:
        path = libclang_finder_instance.get_libclang_path()
        assert path == Path('/custom/path/to/libclang.so')
        mock_cache.assert_called_once_with(Path('/custom/path/to/libclang.so'))


def test_get_libclang_path_with_cached_path(libclang_finder_instance):
    cached_path = Path('/cached/path/to/libclang.so')
    with patch.object(libclang_finder_instance, 'load_cached_libclang_path', return_value=cached_path):
        path = libclang_finder_instance.get_libclang_path()
        assert path == cached_path


def test_get_libclang_path_with_detection(libclang_finder_instance):
    with patch('platform.system', return_value='Linux'), \
            patch.object(libclang_finder_instance, 'find_libclang_linux', return_value=Path('/usr/lib/llvm-18/lib/libclang.so')), \
            patch.object(libclang_finder_instance, 'cache_libclang_path') as mock_cache:
        path = libclang_finder_instance.get_libclang_path()
        assert path == Path('/usr/lib/llvm-18/lib/libclang.so')
        mock_cache.assert_called_once_with(
            Path('/usr/lib/llvm-18/lib/libclang.so'))


def test_get_libclang_path_unsupported_os(libclang_finder_instance):
    with patch('platform.system', return_value='UnsupportedOS'):
        with pytest.raises(RuntimeError, match="Unsupported operating system: UnsupportedOS"):
            libclang_finder_instance.get_libclang_path()


def test_configure_clang(libclang_finder_instance):
    with patch.object(libclang_finder_instance, 'get_libclang_path', return_value=Path('/path/to/libclang.so')), \
            patch('clang.cindex.Config.set_library_file') as mock_set_library_file:
        libclang_finder_instance.configure_clang()
        mock_set_library_file.assert_called_once_with('/path/to/libclang.so')


def test_list_libclang_versions(libclang_finder_instance):
    with patch('platform.system', return_value='Linux'), \
            patch.object(libclang_finder_instance, 'find_libclang_linux', return_value=[Path('/usr/lib/llvm-18/lib/libclang.so')]):
        paths = libclang_finder_instance.list_libclang_versions()
        assert paths == [Path('/usr/lib/llvm-18/lib/libclang.so')]
