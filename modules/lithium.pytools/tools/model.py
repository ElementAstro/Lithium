"""
This module provides classes and functions to generate responses using LLaMA and Ollama models.
"""

import os
import hashlib
import asyncio
from pathlib import Path
from typing import Optional, Dict, Any
import json

from transformers import AutoModelForCausalLM, AutoTokenizer
from loguru import logger


# Define cache directory
CACHE_DIR = Path("./cache")
CACHE_DIR.mkdir(parents=True, exist_ok=True)


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

    def __init__(self, model_name: str = "meta-llama/Llama-3b", device: str = "cpu"):
        """
        Initialize the LLaMA model.

        Args:
            model_name (str): The name of the pre-trained model.
            device (str): Device to run the model on ('cpu' or 'cuda').
        """
        try:
            logger.info(
                f"Loading LLaMA model '{model_name}' on device '{device}'...")
            self.tokenizer = AutoTokenizer.from_pretrained(model_name)
            self.model = AutoModelForCausalLM.from_pretrained(
                model_name).to(device)
            self.device = device
            logger.success(f"LLaMA model '{model_name}' loaded successfully.")
        except Exception as e:
            logger.exception(f"Failed to load LLaMA model '{model_name}': {e}")
            raise

    def generate_response(self, prompt: str, max_length: int = 50) -> str:
        cache_key = self._get_cache_key(prompt, max_length)
        cached_response = self._load_from_cache(cache_key)

        if cached_response:
            logger.debug("Loaded response from cache.")
            return cached_response

        try:
            logger.info(f"Generating response for prompt: {prompt}")
            inputs = self.tokenizer(
                prompt, return_tensors="pt").to(self.device)
            outputs = self.model.generate(
                inputs["input_ids"], max_length=max_length, eos_token_id=self.tokenizer.eos_token_id)
            response = self.tokenizer.decode(
                outputs[0], skip_special_tokens=True)
            self._save_to_cache(cache_key, response)
            logger.debug("Response generated and cached successfully.")
            return response
        except Exception as e:
            logger.exception(f"Failed to generate response: {e}")
            raise

    def _get_cache_key(self, prompt: str, max_length: int) -> str:
        key = f"{prompt}_{max_length}"
        return hashlib.md5(key.encode()).hexdigest()

    def _save_to_cache(self, key: str, response: str) -> None:
        try:
            cache_file = CACHE_DIR / f"{key}.txt"
            with cache_file.open("w", encoding="utf-8") as f:
                f.write(response)
            logger.debug(f"Response saved to cache: {cache_file}")
        except Exception as e:
            logger.error(f"Failed to save response to cache: {e}")

    def _load_from_cache(self, key: str) -> Optional[str]:
        cache_file = CACHE_DIR / f"{key}.txt"
        if cache_file.exists():
            try:
                with cache_file.open("r", encoding="utf-8") as f:
                    logger.debug(
                        f"Loading response from cache file: {cache_file}")
                    return f.read()
            except Exception as e:
                logger.error(f"Failed to load response from cache: {e}")
        return None

    async def generate_response_async(self, prompt: str, max_length: int = 50) -> str:
        loop = asyncio.get_event_loop()
        try:
            response = await loop.run_in_executor(None, self.generate_response, prompt, max_length)
            return response
        except Exception as e:
            logger.exception(f"Async response generation failed: {e}")
            raise


class OllamaModel(BaseModel):
    """
    Ollama model class.
    """

    def __init__(self, model_name: str = "ollama/ollama-3b"):
        """
        Initialize the Ollama model.

        Args:
            model_name (str): The name of the pre-trained Ollama model.
        """
        try:
            logger.info(f"Loading Ollama model '{model_name}'...")
            # Placeholder for actual Ollama model loading
            # self.model = OllamaAPI.load(model_name)
            self.model_name = model_name
            logger.success(f"Ollama model '{model_name}' loaded successfully.")
        except Exception as e:
            logger.exception(
                f"Failed to load Ollama model '{model_name}': {e}")
            raise

    def generate_response(self, prompt: str, max_length: int = 50) -> str:
        try:
            logger.info(
                f"Generating response with Ollama for prompt: {prompt}")
            # Placeholder for actual Ollama response generation
            # response = self.model.generate(prompt, max_length=max_length)
            response = f"Ollama Response to: {prompt} (Simulated)"
            logger.debug("Ollama response generated successfully.")
            return response
        except Exception as e:
            logger.exception(f"Failed to generate Ollama response: {e}")
            raise

    async def generate_response_async(self, prompt: str, max_length: int = 50) -> str:
        try:
            logger.info(
                f"Asynchronously generating response with Ollama for prompt: {prompt}")
            # Placeholder for actual async Ollama response generation
            await asyncio.sleep(1)  # Simulate async operation
            response = self.generate_response(prompt, max_length)
            logger.debug("Ollama async response generated successfully.")
            return response
        except Exception as e:
            logger.exception(f"Async Ollama response generation failed: {e}")
            raise


class ModelManager:
    """
    Model manager class to handle switching between different models.
    """

    def __init__(self, device: str = "cpu"):
        """
        Initialize the ModelManager with available models.

        Args:
            device (str): Device to run models on ('cpu' or 'cuda').
        """
        self.models: Dict[str, BaseModel] = {
            "llama": LLaMAModel(device=device),
            "ollama": OllamaModel()
        }
        self.active_model_name: str = "llama"  # Default model
        logger.info(
            f"ModelManager initialized with active model: {self.active_model_name}")

    def set_active_model(self, model_name: str):
        """
        Set the active model.

        Args:
            model_name (str): The name of the model to set as active.
        """
        model_name = model_name.lower()
        if model_name not in self.models:
            logger.error(f"Model '{model_name}' is not supported.")
            raise ValueError(f"Model '{model_name}' is not supported.")
        self.active_model_name = model_name
        logger.info(f"Active model set to: {self.active_model_name}")

    def generate_response(self, prompt: str, max_length: int = 50) -> str:
        """
        Generate a response using the active model.

        Args:
            prompt (str): The input prompt.
            max_length (int): The maximum length of the generated response.

        Returns:
            str: The generated response.
        """
        try:
            model = self.models[self.active_model_name]
            response = model.generate_response(prompt, max_length)
            logger.debug(
                f"Response generated by {self.active_model_name} model.")
            return response
        except Exception as e:
            logger.exception(f"Failed to generate response: {e}")
            raise

    async def generate_response_async(self, prompt: str, max_length: int = 50) -> str:
        """
        Asynchronously generate a response using the active model.

        Args:
            prompt (str): The input prompt.
            max_length (int): The maximum length of the generated response.

        Returns:
            str: The generated response.
        """
        try:
            model = self.models[self.active_model_name]
            response = await model.generate_response_async(prompt, max_length)
            logger.debug(
                f"Async response generated by {self.active_model_name} model.")
            return response
        except Exception as e:
            logger.exception(f"Failed to generate async response: {e}")
            raise

    def list_available_models(self) -> List[str]:
        """
        List all available models.

        Returns:
            List[str]: Names of available models.
        """
        model_names = list(self.models.keys())
        logger.info(f"Available models: {', '.join(model_names)}")
        return model_names


def cli_interface():
    """
    Command-line interface for generating responses using models.
    """
    parser = argparse.ArgumentParser(
        description="Generate text using LLaMA or Ollama model.")
    parser.add_argument("--prompt", type=str,
                        help="Input prompt for text generation", required=True)
    parser.add_argument("--max_length", type=int, default=50,
                        help="Maximum length of generated text")
    parser.add_argument("--model", type=str, choices=[
                        "llama", "ollama"], default="llama", help="Choose model: LLaMA or Ollama")
    parser.add_argument("--async_mode", action="store_true",
                        help="Generate response asynchronously")
    parser.add_argument("--output", type=str,
                        help="File to save the generated response")
    parser.add_argument("--log_level", type=str, choices=[
                        "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"], default="INFO", help="Set the logging level")
    parser.add_argument(
        "--device", type=str, choices=["cpu", "cuda"], default="cpu", help="Device to run the model on")

    args = parser.parse_args()

    # Configure logging level
    logger.remove()
    logger.add(sys.stderr, level=args.log_level,
               format="<level>{message}</level>")
    logger.add(
        "model.log",
        rotation="10 MB",
        retention="7 days",
        compression="zip",
        enqueue=True,
        encoding="utf-8",
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
        level="DEBUG"
    )
    logger.debug("Logging configured.")

    try:
        model_manager = ModelManager(device=args.device)
        model_manager.set_active_model(args.model)

        if args.async_mode:
            response = asyncio.run(model_manager.generate_response_async(
                args.prompt, args.max_length))
        else:
            response = model_manager.generate_response(
                args.prompt, args.max_length)

        if args.output:
            output_path = Path(args.output)
            with output_path.open("w", encoding="utf-8") as f:
                f.write(response)
            logger.info(f"Response saved to {output_path}")
        else:
            print(f"Response: {response}")

    except Exception as e:
        logger.exception(f"An error occurred during response generation: {e}")
        sys.exit(1)


async def main_async_example():
    """
    Example of an asynchronous call to generate a response.
    """
    try:
        model_manager = ModelManager(
            device="cuda" if torch.cuda.is_available() else "cpu")
        model_manager.set_active_model("ollama")
        prompt = "What are the latest trends in AI research?"
        response = await model_manager.generate_response_async(prompt, max_length=100)
        print(f"Async Response: {response}")
    except Exception as e:
        logger.exception(f"Async example failed: {e}")


if __name__ == "__main__":
    cli_interface()
