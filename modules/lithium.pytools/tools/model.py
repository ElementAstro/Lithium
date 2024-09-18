"""
This module provides classes and functions to generate responses using LLaMA and Ollama models.
"""

import os
import hashlib
import logging
import asyncio
from typing import Optional
from transformers import AutoModelForCausalLM, AutoTokenizer

# 配置日志
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(levelname)s - %(message)s')

# 定义缓存目录
CACHE_DIR = "./cache"
if not os.path.exists(CACHE_DIR):
    os.makedirs(CACHE_DIR)


class BaseModel:
    """
    Abstract base class for models.
    """

    def generate_response(self, prompt: str, max_length: int = 50) -> str:
        """
        Generate a response for the given prompt.

        Args:
            prompt (str): The input prompt.
            max_length (int): The maximum length of the generated response.

        Returns:
            str: The generated response.
        """
        raise NotImplementedError

    async def generate_response_async(self, prompt: str, max_length: int = 50) -> str:
        """
        Asynchronously generate a response for the given prompt.

        Args:
            prompt (str): The input prompt.
            max_length (int): The maximum length of the generated response.

        Returns:
            str: The generated response.
        """
        raise NotImplementedError


class LLaMAModel(BaseModel):
    """
    LLaMA model class.
    """

    def __init__(self, model_name: str = "meta-llama/Llama-3b"):
        logging.info("Loading LLaMA model %s...", model_name)
        self.tokenizer = AutoTokenizer.from_pretrained(model_name)
        self.model = AutoModelForCausalLM.from_pretrained(model_name)

    def generate_response(self, prompt: str, max_length: int = 50) -> str:
        cache_key = self._get_cache_key(prompt, max_length)
        cached_response = self._load_from_cache(cache_key)

        if cached_response:
            logging.info("Loaded response from cache.")
            return cached_response

        logging.info("Generating response for prompt: %s", prompt)
        inputs = self.tokenizer(prompt, return_tensors="pt")
        outputs = self.model.generate(
            inputs["input_ids"], max_length=max_length)
        response = self.tokenizer.decode(outputs[0], skip_special_tokens=True)

        self._save_to_cache(cache_key, response)
        return response

    def _get_cache_key(self, prompt: str, max_length: int) -> str:
        key = f"{prompt}_{max_length}"
        return hashlib.md5(key.encode()).hexdigest()

    def _save_to_cache(self, key: str, response: str):
        with open(f"{CACHE_DIR}/{key}.txt", "w", encoding="utf-8") as f:
            f.write(response)

    def _load_from_cache(self, key: str) -> Optional[str]:
        file_path = f"{CACHE_DIR}/{key}.txt"
        if os.path.exists(file_path):
            with open(file_path, "r", encoding="utf-8") as f:
                return f.read()
        return None

    async def generate_response_async(self, prompt: str, max_length: int = 50) -> str:
        loop = asyncio.get_event_loop()
        response = await loop.run_in_executor(None, self.generate_response, prompt, max_length)
        return response


class OllamaModel(BaseModel):
    """
    Ollama model class.
    """

    def __init__(self, model_name: str = "ollama/ollama-3b"):
        logging.info("Loading Ollama model %s...", model_name)
        # 模拟Ollama模型的加载过程
        # 假设有类似接口
        # self.model = OllamaModelAPI.load(model_name) # 如果Ollama有自己的API，使用此处加载

    def generate_response(self, prompt: str, max_length: int = 50) -> str:
        logging.info("Generating response with Ollama for prompt: %s", prompt)
        # 假设有类似接口生成响应
        response = f"Ollama Response to: {prompt} (Simulated)"
        return response

    async def generate_response_async(self, prompt: str, max_length: int = 50) -> str:
        logging.info(
            "Generating async response with Ollama for prompt: %s", prompt)
        # 模拟异步调用 Ollama 模型
        await asyncio.sleep(1)  # 假设异步调用需要一定的时间
        return self.generate_response(prompt, max_length)


class ModelManager:
    """
    Model manager class to handle switching between different models.
    """

    def __init__(self):
        self.models = {
            "llama": LLaMAModel(),
            "ollama": OllamaModel()
        }
        self.active_model = "llama"  # 默认使用LLaMA模型

    def set_active_model(self, model_name: str):
        """
        Set the active model.

        Args:
            model_name (str): The name of the model to set as active.
        """
        if model_name not in self.models:
            raise ValueError(f"Model {model_name} is not supported.")
        self.active_model = model_name
        logging.info("Switched to %s model.", model_name)

    def generate_response(self, prompt: str, max_length: int = 50) -> str:
        """
        Generate a response using the active model.

        Args:
            prompt (str): The input prompt.
            max_length (int): The maximum length of the generated response.

        Returns:
            str: The generated response.
        """
        model = self.models[self.active_model]
        return model.generate_response(prompt, max_length)

    async def generate_response_async(self, prompt: str, max_length: int = 50) -> str:
        """
        Asynchronously generate a response using the active model.

        Args:
            prompt (str): The input prompt.
            max_length (int): The maximum length of the generated response.

        Returns:
            str: The generated response.
        """
        model = self.models[self.active_model]
        return await model.generate_response_async(prompt, max_length)


def cli_interface():
    import argparse
    parser = argparse.ArgumentParser(
        description="Generate text using LLaMA or Ollama model.")
    parser.add_argument("--prompt", type=str,
                        help="Input prompt for text generation", required=True)
    parser.add_argument("--max_length", type=int, default=50,
                        help="Maximum length of generated text")
    parser.add_argument("--model", type=str, choices=[
                        "llama", "ollama"], default="llama", help="Choose model: LLaMA or Ollama")
    parser.add_argument("--async_mode", action="store_true",
                        help="Generate response asynchronously")  # 修改了参数名
    parser.add_argument("--output", type=str,
                        help="File to save the generated response")
    parser.add_argument("--log_level", type=str, choices=[
                        "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"], default="INFO", help="Set the logging level")

    args = parser.parse_args()

    # 设置日志级别
    logging.getLogger().setLevel(args.log_level)

    model_manager = ModelManager()
    model_manager.set_active_model(args.model)

    if args.async_mode:  # 修改了参数名
        response = asyncio.run(model_manager.generate_response_async(
            args.prompt, args.max_length))
    else:
        response = model_manager.generate_response(
            args.prompt, args.max_length)

    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(response)
        logging.info("Response saved to %s", args.output)
    else:
        print(f"Response: {response}")


async def main_async():
    """
    Example of asynchronous call to generate a response.
    """
    model_manager = ModelManager()
    model_manager.set_active_model("ollama")  # 切换到 Ollama 模型
    prompt = "What are the latest trends in AI research?"
    response = await model_manager.generate_response_async(prompt)
    print(f"Async Response: {response}")


if __name__ == "__main__":
    cli_interface()
